#pragma once

#include <Windows.h>
#include <atomic>
#include <thread>

class ProcessManager
{
  public:
    ProcessManager();
    ~ProcessManager();

    bool Start();

    void Stop();

    bool IsRunning() const;

    DWORD GetProcessId() const
    {
        return process_id_;
    }

    HANDLE GetProcessHandle() const
    {
        return process_handle_;
    }

  private:
    std::atomic<HANDLE> process_handle_{nullptr};
    std::atomic<DWORD> process_id_{0};

    std::thread monitor_thread_;
    std::atomic_bool monitor_running_{false};

    void MonitorThreadProc();
};
