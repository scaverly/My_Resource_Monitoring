#pragma once
#include "lib.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

struct ProcessInfo {
    std::wstring name;
    DWORD pid;
    DWORD threads;
    size_t memoryUsage;
};  

class ProcessMonitor {
public:
    void Refresh();
    void Display(); // Для отладки в консоли
    void UpdateListView(HWND hListView, HWND hSearchEdit);

private:
    std::vector<ProcessInfo> m_processes;
    size_t GetMemoryUsage(DWORD processID);
};