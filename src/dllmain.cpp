#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <cstdint>
#include "Config.h"
#include "core/ConfigLoader.h"
#include "core/GameBase.h"
#include "core/EntityList.h"
#include "features/WaitHack.h"
#include "gui/GUI.h"
#include "features/MetinFarm.h"

// ============================================================
// BOT THREAD
// ============================================================
DWORD WINAPI BotThread(LPVOID) {
    while (true) {
        Sleep(50);
        __try {
            uintptr_t playerBase = GameBase::GetPlayerBase();
            uintptr_t charMgr    = GameBase::GetCharMgrBase();
            uintptr_t netStream  = GameBase::GetNetStreamBase();
            if (!playerBase || !charMgr || !netStream) continue;

            uint32_t  mainVID = GameBase::GetMainActorVID(netStream);
            uintptr_t inst    = EntityList::FindInstanceByVID(charMgr, mainVID);
            if (!inst) continue;

            float px = 0, py = 0;
            GameBase::GetRealPosition(inst, &px, &py);

        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
    return 0;
}

// ============================================================
// Metin Farm THREAD
// ============================================================
DWORD WINAPI MetinFarmThread(LPVOID) {
    while (true) {
        Sleep(500);  // aumenta a 500ms
        if (!g_config.metinFarm || !g_config.botActive) continue;
        __try {
            uintptr_t playerBase = GameBase::GetPlayerBase();
            uintptr_t charMgr = GameBase::GetCharMgrBase();
            if (!playerBase || !charMgr) continue;
            MetinFarm::OnUpdate(playerBase, charMgr);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    return 0;
}
// ============================================================
// WAITHACK THREAD
// ============================================================
DWORD WINAPI WaitHackThread(LPVOID) {
    while (true) {
        Sleep(50);
        if (!g_config.waithack || !g_config.botActive) continue;
        __try {
            uintptr_t playerBase = GameBase::GetPlayerBase();
            uintptr_t charMgr = GameBase::GetCharMgrBase();
            uintptr_t netStream = GameBase::GetNetStreamBase();
            if (!playerBase || !charMgr || !netStream) continue;

            uint32_t  mainVID = GameBase::GetMainActorVID(netStream);
            uintptr_t inst = EntityList::FindInstanceByVID(charMgr, mainVID);
            if (!inst) continue;

            float px = 0, py = 0;
            GameBase::GetRealPosition(inst, &px, &py);

            WaitHack::AttackAllNearby(charMgr, px, py,
                g_config.whRange, g_config.whDelay);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    return 0;
}

// ============================================================
// MAIN THREAD
// ============================================================
void MainThread(HMODULE hModule) {
    // Carica config.json (dalla stessa cartella della DLL)
    // Se non trovato usa i valori default hardcodati
    ConfigLoader::Load();

    // Attendi inizializzazione singleton
    while (!GameBase::GetPlayerBase()) Sleep(500);
    while (!GameBase::GetCharMgrBase()) Sleep(500);

    CreateDirectoryA("C:/py", nullptr);

    CreateThread(nullptr, 0, GUI::Thread,  nullptr, 0, nullptr);
    CreateThread(nullptr, 0, BotThread,    nullptr, 0, nullptr);
    CreateThread(nullptr, 0, WaitHackThread, nullptr, 0, nullptr);
    CreateThread(nullptr, 0, MetinFarmThread, nullptr, 0, nullptr);

    while (true) Sleep(1000);
}

// ============================================================
// DLL ENTRY POINT
// ============================================================
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0,
            (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
    }
    return TRUE;
}