#pragma once
#include <Windows.h>

namespace win {
    BOOL CALLBACK SendWMCloseMsg(HWND hwnd, LPARAM lParam)
    {
        DWORD dwProcessId = 0;
        GetWindowThreadProcessId(hwnd, &dwProcessId);
        if (dwProcessId == lParam)
            SendMessageTimeout(hwnd, WM_CLOSE, 0, 0, SMTO_ABORTIFHUNG, 30000, NULL);
        return TRUE;
    }
}