#include "ProcessManager.hpp"

#include <format>

#include "Logger.hpp"
#include "RuntimePaths.hpp"

ProcessManager::ProcessManager() = default;

ProcessManager::~ProcessManager()
{
    Stop();
}

bool ProcessManager::Start()
{
    if (IsRunning())
    {
        Logger::Instance().Log("Subprocess already running (PID: {})", process_id_.load());

        return true;
    }

    const auto gameDirectory = luacef::paths::ExecutableDirectory();
    const auto hostPath = luacef::paths::FindHost(luacef::paths::HostEnvironmentDirectory(), gameDirectory);

    if (hostPath.empty())
    {
        Logger::Instance().Log("cef-host.exe not found in LUACEF_HOST_PATH or the game directory");

        return false;
    }

    const auto hostPathUtf8 = hostPath.u8string();
    Logger::Instance().Log("Host path: {}", std::string(hostPathUtf8.begin(), hostPathUtf8.end()));

    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESHOWWINDOW;
#ifdef NDEBUG
    startup_info.wShowWindow = SW_HIDE;
#endif

    PROCESS_INFORMATION process_info{};

    const auto parentPid = GetCurrentProcessId();
    auto cmdLine = std::format(L"\"{}\" --parent-pid={}", hostPath.native(), parentPid);

    if (!CreateProcessW(hostPath.c_str(), cmdLine.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr,
                        gameDirectory.empty() ? nullptr : gameDirectory.c_str(), &startup_info, &process_info))
    {
        Logger::Instance().Log("CreateProcess failed: {}", GetLastError());

        return false;
    }

    process_handle_ = process_info.hProcess;
    process_id_ = process_info.dwProcessId;

    CloseHandle(process_info.hThread);

    Logger::Instance().Log("Subprocess created (PID: {})", process_id_.load());

    monitor_running_ = true;

    monitor_thread_ = std::thread([this]() { MonitorThreadProc(); });

    return true;
}

void ProcessManager::Stop()
{
    monitor_running_ = false;

    if (monitor_thread_.joinable())
    {
        monitor_thread_.join();
    }

    if (!process_handle_)
        return;

    Logger::Instance().Log("Stopping subprocess (PID: {})", process_id_.load());

    DWORD exit_code{};

    if (GetExitCodeProcess(process_handle_, &exit_code) && exit_code == STILL_ACTIVE)
    {
        WaitForSingleObject(process_handle_, 1000);

        GetExitCodeProcess(process_handle_, &exit_code);

        if (exit_code == STILL_ACTIVE)
        {
            Logger::Instance().Log("Force terminating subprocess");

            TerminateProcess(process_handle_, 0);

            WaitForSingleObject(process_handle_, 5000);
        }
    }

    CloseHandle(process_handle_);

    process_handle_ = nullptr;
    process_id_ = 0;

    Logger::Instance().Log("Subprocess stopped");
}

bool ProcessManager::IsRunning() const
{
    if (process_handle_ == nullptr)
        return false;

    DWORD exit_code = 0;
    if (!GetExitCodeProcess(process_handle_, &exit_code))
        return false;

    return exit_code == STILL_ACTIVE;
}

void ProcessManager::MonitorThreadProc()
{
    while (monitor_running_)
    {
        HANDLE process = process_handle_;

        if (!process)
            break;

        DWORD exit_code{};

        if (!GetExitCodeProcess(process, &exit_code))
        {
            Logger::Instance().Log("GetExitCodeProcess failed");

            break;
        }

        if (exit_code != STILL_ACTIVE)
        {
            Logger::Instance().Log("Subprocess exited with code {}", exit_code);

            break;
        }

        Sleep(500);
    }
}
