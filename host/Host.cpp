#include "Host.hpp"

#include <filesystem>
#include <format>
#include <thread>

#include <Windows.h>
#include <cef_app.h>

#include "Logger.hpp"
#include "RuntimePaths.hpp"

Host::~Host()
{
    if (serverThread_.running)
    {
        serverThread_.running = false;
        serverThread_.thread.join();
    }

    if (parentMonitorThread_.running)
    {
        parentMonitorThread_.running = false;
        parentMonitorThread_.thread.join();
    }
}

bool Host::Initialize(DWORD pid)
{
    parentPid_ = pid;
    InitCef();

    ipcHandler_ = std::make_shared<IPCBrowserHandler>(parentPid_);
    ipcServer_ = std::make_shared<ipc::IPCServer>(ipcHandler_);
    ipcHandler_->SetIPCServer(ipcServer_);

    // Запустить сервер в отдельном потоке
    serverThread_.thread =
        std::thread([this]() { ipcServer_->Start(std::format(R"(\\.\pipe\LuaCEF_IPC_{})", parentPid_), 8); });

    StartParentMonitor();

    return true;
}

void Host::Run()
{
    Sleep(INFINITE);
}

void Host::InitCef()
{
    const auto runtimePath = luacef::paths::ExecutableDirectory();
    const auto dataPath = std::filesystem::current_path() / L"luacef";
    std::filesystem::create_directories(dataPath / L"cache");

    CefSettings settings;

    CefString(&settings.browser_subprocess_path) = (runtimePath / L"cef-subprocess.exe").wstring();
    CefString(&settings.log_file) = (dataPath / L"luacef.log").wstring();
    CefString(&settings.cache_path) = (dataPath / L"cache").wstring();
    CefString(&settings.locales_dir_path) = (runtimePath / L"locales").wstring();
    CefString(&settings.resources_dir_path) = runtimePath.wstring();
    CefString(&settings.locale).FromASCII("ru");
    CefString(&settings.accept_language_list).FromASCII("en-US,en,ru-RU,ru");

    settings.no_sandbox = 1;
    settings.multi_threaded_message_loop = 1;
    settings.windowless_rendering_enabled = 1;

    settings.background_color = 0;
    settings.log_severity = LOGSEVERITY_DEBUG;
    settings.command_line_args_disabled = 1;

#ifndef NDEBUG
    settings.remote_debugging_port = 9222;
#endif

    CefMainArgs args(GetModuleHandle(nullptr));

    CefInitialize(args, settings, nullptr, nullptr);
}

void Host::StartParentMonitor()
{
    if (parentPid_ == 0)
        return;

    parentMonitorThread_.thread = std::thread([this]() { ParentMonitorProc(); });
}

void Host::ParentMonitorProc()
{
    Logger::Instance().Log("Parent monitor started. PID={}", parentPid_);

    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, parentPid_);

    if (!process)
        ExitProcess(0);

    WaitForSingleObject(process, INFINITE);

    CloseHandle(process);

    Logger::Instance().Log("Host shutting down because parent died");

    CefShutdown();

    ExitProcess(0);
}
