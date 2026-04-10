#include "Monitor.h"

size_t ProcessMonitor::GetMemoryUsage(DWORD processID) {
    size_t mem = 0;
    // Открываем Handle процесса только для чтения информации
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

    if (hProcess) {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            mem = pmc.WorkingSetSize / 1024;
        }
        CloseHandle(hProcess); // Обязательно закрываем Handle
    }
    return mem;
}

void ProcessMonitor::Refresh() {
    m_processes.clear();

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(pe32);

    if (Process32FirstW(hSnap, &pe32)) {
        do {
            ProcessInfo info;
            info.name = pe32.szExeFile;
            info.pid = pe32.th32ProcessID;
            info.threads = pe32.cntThreads;
            info.memoryUsage = GetMemoryUsage(pe32.th32ProcessID);

            m_processes.push_back(info);
        } while (Process32NextW(hSnap, &pe32));
    }
    CloseHandle(hSnap);
}

void ProcessMonitor::Display() {
    system("cls");
    std::wcout << std::left << std::setw(35) << L"Process Name"
        << std::setw(10) << L"PID"
        << std::setw(12) << L"Threads"
        << L"Memory (KB)" << std::endl;
    std::wcout << std::wstring(70, L'-') << std::endl;

    for (const auto& proc : m_processes) {
        std::wcout << std::left << std::setw(35) << proc.name
            << std::setw(10) << proc.pid
            << std::setw(12) << proc.threads
            << proc.memoryUsage << std::endl;
    }
}

void ProcessMonitor::UpdateListView(HWND hListView, HWND hSearchEdit) {
    // Получаем текст из поиска и переводим в нижний регистр
    wchar_t filter[256];
    GetWindowText(hSearchEdit, filter, 256);
    std::wstring searchStr(filter);
    std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), std::towlower);

    // Запоминаем текущую позицию скролла
    int topIndex = ListView_GetTopIndex(hListView);

    SendMessage(hListView, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(hListView);

    for (const auto& proc : m_processes) {
        // Фильтрация
        if (!searchStr.empty()) {
            std::wstring nameLower = proc.name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), std::towlower);
            if (nameLower.find(searchStr) == std::wstring::npos) continue;
        }

        // Добавление
        LVITEM lvi = { 0 };
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = ListView_GetItemCount(hListView);
        lvi.pszText = (LPWSTR)proc.name.c_str();
        lvi.lParam = (LPARAM)proc.pid;
        int pos = ListView_InsertItem(hListView, &lvi);

        wchar_t buf[256];
        swprintf(buf, 256, L"%d", proc.pid);
        ListView_SetItemText(hListView, pos, 1, buf);
        swprintf(buf, 256, L"%d", proc.threads);
        ListView_SetItemText(hListView, pos, 2, buf);
        swprintf(buf, 256, L"%zu KB", proc.memoryUsage);
        ListView_SetItemText(hListView, pos, 3, buf);
    }

    // Возвращаем скролл на место (только если поиск пустой, чтобы не мешать фильтрации)
    if (searchStr.empty()) {
        RECT rc;
        if (ListView_GetItemRect(hListView, 0, &rc, LVIR_BOUNDS)) {
            int itemHeight = rc.bottom - rc.top;
            ListView_Scroll(hListView, 0, topIndex * itemHeight);
        }
    }

    SendMessage(hListView, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hListView, NULL, TRUE);
}