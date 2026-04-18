#pragma once
#include <Windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <cstdio>
#include "../Config.h"
#include "../core/Memory.h"
#include "../core/GameBase.h"
#include "../core/EntityList.h"
#include "../core/Pointers.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dwmapi.lib")

namespace GUI {

    enum ControlID {
        ID_CHK_WAIT = 101,
        ID_BTN_BOT = 102,
        ID_LBL_STATUS = 103,
        ID_SLIDER_DELAY = 105,
        ID_LBL_DELAY = 106,
        ID_SLIDER_RANGE = 107,
        ID_LBL_RANGE = 108,
        ID_CHK_METIN = 109,
        ID_CHK_WH_MOB = 112,
        ID_CHK_WH_METIN = 113,
        ID_TIMER = 200,
    };

    inline HWND g_hWnd = nullptr;
    inline HWND g_hLblStatus = nullptr;
    inline HWND g_hBtnBot = nullptr;
    inline HWND g_hSliderDelay = nullptr;
    inline HWND g_hLblDelay = nullptr;
    inline HWND g_hSliderRange = nullptr;
    inline HWND g_hLblRange = nullptr;
    inline HWND g_hChkMetin = nullptr;

    inline LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_HSCROLL: {
            if ((HWND)lParam == g_hSliderDelay) {
                int pos = (int)SendMessageA(g_hSliderDelay, TBM_GETPOS, 0, 0);
                g_config.whDelay = pos;
                char buf[32];
                sprintf_s(buf, "Delay: %d ms", pos);
                SetWindowTextA(g_hLblDelay, buf);
            }
            else if ((HWND)lParam == g_hSliderRange) {
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
            case ID_CHK_WH_MOB:
                g_config.whAttackMob = !g_config.whAttackMob;
                SendMessageA((HWND)lParam, BM_SETCHECK,
                    g_config.whAttackMob ? BST_CHECKED : BST_UNCHECKED, 0);
                break;
            case ID_CHK_WH_METIN:
                g_config.whAttackMetin = !g_config.whAttackMetin;
                SendMessageA((HWND)lParam, BM_SETCHECK,
                    g_config.whAttackMetin ? BST_CHECKED : BST_UNCHECKED, 0);
                break;
            case ID_BTN_BOT:
                g_config.botActive = !g_config.botActive;
                SetWindowTextA(g_hBtnBot,
                    g_config.botActive ? "Bot: ON" : "Bot: OFF");
                break;
            case ID_CHK_METIN:
                g_config.metinFarm = !g_config.metinFarm;
                SendMessageA((HWND)lParam, BM_SETCHECK,
                    g_config.metinFarm ? BST_CHECKED : BST_UNCHECKED, 0);
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

    inline DWORD WINAPI Thread(LPVOID) {
        INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_WIN95_CLASSES };
        InitCommonControlsEx(&icex);

        WNDCLASSEXA wc = {};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hbrBackground = CreateSolidBrush(RGB(20, 20, 20));
        wc.lpszClassName = "M2BotWnd";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClassExA(&wc);

        g_hWnd = CreateWindowExA(
            WS_EX_TOPMOST,
            "M2BotWnd", "FarmBot",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            100, 100, 270, 370,
            NULL, NULL, GetModuleHandle(NULL), NULL
        );

        BOOL dark = TRUE;
        DwmSetWindowAttribute(g_hWnd, 20, &dark, sizeof(dark));

        // ── Waithack ──────────────────────────────────────
        HWND hChkWait = CreateWindowExA(0, "BUTTON", "Waithack",
            WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
            12, 12, 120, 22, g_hWnd, (HMENU)ID_CHK_WAIT, NULL, NULL);
        SendMessageA(hChkWait, BM_SETCHECK, BST_UNCHECKED, 0);

        // Target: Mob / Metin
        HWND hChkWhMob = CreateWindowExA(0, "BUTTON", "Mob",
            WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
            24, 38, 70, 18, g_hWnd, (HMENU)ID_CHK_WH_MOB, NULL, NULL);
        SendMessageA(hChkWhMob, BM_SETCHECK, BST_CHECKED, 0);

        HWND hChkWhMetin = CreateWindowExA(0, "BUTTON", "Metin",
            WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
            100, 38, 80, 18, g_hWnd, (HMENU)ID_CHK_WH_METIN, NULL, NULL);
        SendMessageA(hChkWhMetin, BM_SETCHECK, BST_UNCHECKED, 0);

        g_hLblDelay = CreateWindowExA(0, "STATIC", "Delay: 10 ms",
            WS_CHILD | WS_VISIBLE,
            12, 62, 120, 18, g_hWnd, (HMENU)ID_LBL_DELAY, NULL, NULL);

        g_hSliderDelay = CreateWindowExA(0, TRACKBAR_CLASSA, "",
            WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
            12, 80, 235, 28, g_hWnd, (HMENU)ID_SLIDER_DELAY, NULL, NULL);
        SendMessageA(g_hSliderDelay, TBM_SETRANGE, TRUE, MAKELPARAM(0, 500));
        SendMessageA(g_hSliderDelay, TBM_SETPOS, TRUE, 10);
        SendMessageA(g_hSliderDelay, TBM_SETTICFREQ, 100, 0);

        g_hLblRange = CreateWindowExA(0, "STATIC", "Range: 1500",
            WS_CHILD | WS_VISIBLE,
            12, 114, 120, 18, g_hWnd, (HMENU)ID_LBL_RANGE, NULL, NULL);

        g_hSliderRange = CreateWindowExA(0, TRACKBAR_CLASSA, "",
            WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
            12, 132, 235, 28, g_hWnd, (HMENU)ID_SLIDER_RANGE, NULL, NULL);
        SendMessageA(g_hSliderRange, TBM_SETRANGE, TRUE, MAKELPARAM(1, 30));
        SendMessageA(g_hSliderRange, TBM_SETPOS, TRUE, 15);
        SendMessageA(g_hSliderRange, TBM_SETTICFREQ, 5, 0);

        // ── Separatore ────────────────────────────────────
        CreateWindowExA(0, "STATIC", "",
            WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
            12, 170, 235, 2, g_hWnd, nullptr, NULL, NULL);

        // ── Metin Farm ────────────────────────────────────
        g_hChkMetin = CreateWindowExA(0, "BUTTON", "Metin Farm",
            WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
            12, 180, 120, 22, g_hWnd, (HMENU)ID_CHK_METIN, NULL, NULL);
        SendMessageA(g_hChkMetin, BM_SETCHECK, BST_UNCHECKED, 0);

        // ── Separatore ────────────────────────────────────
        CreateWindowExA(0, "STATIC", "",
            WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
            12, 212, 235, 2, g_hWnd, nullptr, NULL, NULL);

        // ── Bot ON/OFF ────────────────────────────────────
        g_hBtnBot = CreateWindowExA(0, "BUTTON", "Bot: OFF",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            12, 222, 235, 28, g_hWnd, (HMENU)ID_BTN_BOT, NULL, NULL);

        // ── Status ────────────────────────────────────────
        g_hLblStatus = CreateWindowExA(0, "STATIC", "Waithack: OFF",
            WS_CHILD | WS_VISIBLE,
            12, 262, 220, 18, g_hWnd, (HMENU)ID_LBL_STATUS, NULL, NULL);

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