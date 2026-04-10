#include "Monitor.h"

ProcessMonitor g_monitor;
HWND hListView;
HWND hSearchEdit;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // Инициализация системных визуальных стилей
        INITCOMMONCONTROLSEX icex = { 0 };
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&icex);

        // Создаем таблицу процессов
        // Оставляем сверху 40 пикселей под панель поиска
        hListView = CreateWindowEx(0, WC_LISTVIEW, L"",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_BORDER,
            0, 40, 800, 560, hwnd, (HMENU)1, NULL, NULL);

        // Включение расширенных стилей: выделение всей строки и сетку
        ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        // Добавление колонки
        LVCOLUMN lvc = { 0 };
        lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
        lvc.fmt = LVCFMT_LEFT;

        const wchar_t* columnNames[] = { L"Name", L"PID", L"Threads", L"Memory" };
        int columnWidths[] = { 250, 80, 100, 120 };

        for (int i = 0; i < 4; i++) {
            lvc.iSubItem = i;
            lvc.pszText = (LPWSTR)columnNames[i];
            lvc.cx = columnWidths[i];
            ListView_InsertColumn(hListView, i, &lvc);
        }

        // Поле поиска
        hSearchEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            580, 8, 200, 24, hwnd, (HMENU)2, NULL, NULL);

        // Текстовую подсказка в поле
        SendMessage(hSearchEdit, EM_SETCUEBANNER, FALSE, (LPARAM)L"Поиск по названию...");

        // Системный шрифт
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(hListView, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hSearchEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Таймер обновления
        SetTimer(hwnd, 1, 1000, NULL);

        break;
    }

    case WM_SIZE: {
        int newWidth = LOWORD(lParam);
        int newHeight = HIWORD(lParam);

        if (hListView) {
            MoveWindow(hListView, 0, 40, newWidth, newHeight - 40, TRUE);
        }
        if (hSearchEdit) {
            MoveWindow(hSearchEdit, newWidth - 210, 8, 200, 24, TRUE);
        }
        return 0;
    }

    case WM_TIMER:
        g_monitor.Refresh();
        g_monitor.UpdateListView(hListView, hSearchEdit);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 2 && HIWORD(wParam) == EN_CHANGE) {
            g_monitor.UpdateListView(hListView, hSearchEdit);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
    // Регистрация класса
    const wchar_t CLASS_NAME[] = L"ResourceMonitorClass";
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    // Создание окна
    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"My Resource Monitor",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    // Цикл сообщений
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}