#pragma once
#include <Windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <cstdio>
#include "../Config.h"
#include "../core/GameBase.h"


#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dwmapi.lib")

namespace GUI {

    // Control IDs
    enum ControlID {
        ID_CHK_WAIT     = 101,
        ID_BTN_BOT      = 102,
        ID_LBL_STATUS   = 103,
        ID_LBL_POS      = 104,
        ID_SLIDER_DELAY = 105,
        ID_LBL_DELAY    = 106,
        ID_SLIDER_RANGE = 107,
        ID_LBL_RANGE    = 108,
        ID_TIMER        = 200,
    };

    inline HWND g_hWnd         = nullptr;
    inline HWND g_hLblStatus   = nullptr;
    inline HWND g_hLblPos      = nullptr;
    inline HWND g_hBtnBot      = nullptr;
    inline HWND g_hSliderDelay = nullptr;
    inline HWND g_hLblDelay    = nullptr;
    inline HWND g_hSliderRange = nullptr;
    inline HWND g_hLblRange    = nullptr;

    inline LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_HSCROLL: {
            if ((HWND)lParam == g_hSliderDelay) {
                int pos = (int)SendMessageA(g_hSliderDelay, TBM_GETPOS, 0, 0);
                g_config.whDelay = pos;
                char buf[32];
                sprintf_s(buf, "Delay: %d ms", pos);
                SetWindowTextA(g_hLblDelay, buf);
            } else if ((HWND)lParam == g_hSliderRange) {
                int pos = (int)SendMessageA(g_hSliderRange, TBM_GETPOS, 0, 0);
                g_config.whRange = (float)(pos * 100);
                char buf[32];
                sprintf_s(buf, "Range: %d", pos * 100);
                SetWindowTextA(g_hLblRange, buf);
            }
            break;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
            case ID_CHK_WAIT:
                g_config.waithack = !g_config.waithack;
                SendMessageA((HWND)lParam, BM_SETCHECK,
                    g_config.waithack ? BST_CHECKED : BST_UNCHECKED, 0);
                SetWindowTextA(g_hLblStatus,
                    g_config.waithack ? "Waithack: ON" : "Waithack: OFF");
                break;
            case ID_BTN_BOT:
                g_config.botActive = !g_config.botActive;
                SetWindowTextA(g_hBtnBot,
                    g_config.botActive ? "Bot: ON" : "Bot: OFF");
                break;
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(200, 200, 200));
            SetBkColor(hdc, RGB(20, 20, 20));
            return (LRESULT)CreateSolidBrush(RGB(20, 20, 20));
        }
        case WM_CTLCOLORBTN: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(200, 200, 200));
            SetBkColor(hdc, RGB(40, 40, 40));
            return (LRESULT)CreateSolidBrush(RGB(40, 40, 40));
        }
        case WM_DESTROY:
            KillTimer(hWnd, ID_TIMER);
            PostQuitMessage(0);
            break;
        }
        return DefWindowProcA(hWnd, msg, wParam, lParam);
    }

    inline void UpdatePosition(float px, float py) {
        if (!g_hLblPos) return;
        char buf[64];
        sprintf_s(buf, "Pos: %.0f / %.0f", px, py);
        SetWindowTextA(g_hLblPos, buf);
    }

    inline DWORD WINAPI Thread(LPVOID) {
        INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_WIN95_CLASSES };
        InitCommonControlsEx(&icex);

        WNDCLASSEXA wc = {};
        wc.cbSize        = sizeof(wc);
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = GetModuleHandle(NULL);
        wc.hbrBackground = CreateSolidBrush(RGB(20, 20, 20));
        wc.lpszClassName = "M2BotWnd";
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        RegisterClassExA(&wc);

        g_hWnd = CreateWindowExA(
            WS_EX_TOPMOST,
            "M2BotWnd", "Metin2 Bot",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            100, 100, 270, 300,
            NULL, NULL, GetModuleHandle(NULL), NULL
        );

        BOOL dark = TRUE;
        DwmSetWindowAttribute(g_hWnd, 20, &dark, sizeof(dark));

        // Waithack checkbox
        HWND hChkWait = CreateWindowExA(0, "BUTTON", "Waithack",
            WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
            12, 12, 120, 22, g_hWnd, (HMENU)ID_CHK_WAIT, NULL, NULL);
        SendMessageA(hChkWait, BM_SETCHECK, BST_UNCHECKED, 0);

        // Delay label + slider
        g_hLblDelay = CreateWindowExA(0, "STATIC", "Delay: 10 ms",
            WS_CHILD | WS_VISIBLE,
            12, 42, 120, 18, g_hWnd, (HMENU)ID_LBL_DELAY, NULL, NULL);

        g_hSliderDelay = CreateWindowExA(0, TRACKBAR_CLASSA, "",
            WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
            12, 62, 235, 28, g_hWnd, (HMENU)ID_SLIDER_DELAY, NULL, NULL);
        SendMessageA(g_hSliderDelay, TBM_SETRANGE, TRUE, MAKELPARAM(0, 500));
        SendMessageA(g_hSliderDelay, TBM_SETPOS, TRUE, 10);
        SendMessageA(g_hSliderDelay, TBM_SETTICFREQ, 100, 0);

        // Range label + slider
        g_hLblRange = CreateWindowExA(0, "STATIC", "Range: 1500",
            WS_CHILD | WS_VISIBLE,
            12, 98, 120, 18, g_hWnd, (HMENU)ID_LBL_RANGE, NULL, NULL);

        g_hSliderRange = CreateWindowExA(0, TRACKBAR_CLASSA, "",
            WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
            12, 118, 235, 28, g_hWnd, (HMENU)ID_SLIDER_RANGE, NULL, NULL);
        SendMessageA(g_hSliderRange, TBM_SETRANGE, TRUE, MAKELPARAM(1, 30));
        SendMessageA(g_hSliderRange, TBM_SETPOS, TRUE, 15);
        SendMessageA(g_hSliderRange, TBM_SETTICFREQ, 5, 0);

        // Bot button
        g_hBtnBot = CreateWindowExA(0, "BUTTON", "Bot: OFF",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            12, 156, 235, 28, g_hWnd, (HMENU)ID_BTN_BOT, NULL, NULL);

        // Status + pos labels
        g_hLblStatus = CreateWindowExA(0, "STATIC", "Waithack: OFF",
            WS_CHILD | WS_VISIBLE,
            12, 194, 220, 18, g_hWnd, (HMENU)ID_LBL_STATUS, NULL, NULL);

        g_hLblPos = CreateWindowExA(0, "STATIC", "Pos: 0 / 0",
            WS_CHILD | WS_VISIBLE,
            12, 216, 220, 18, g_hWnd, (HMENU)ID_LBL_POS, NULL, NULL);

        ShowWindow(g_hWnd, SW_SHOW);
        UpdateWindow(g_hWnd);
        SetTimer(g_hWnd, ID_TIMER, 500, NULL);

        MSG msg;
        while (GetMessageA(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        return 0;
    }

} // namespace GUI
