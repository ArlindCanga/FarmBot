#pragma once
#include <Windows.h>
#include <cstdint>
#include "../core/Memory.h"
#include "../core/GameBase.h"
#include "../core/EntityList.h"
#include "../core/Pointers.h"
#include "../Config.h"

namespace WaitHack {

    // Invia direttamente il pacchetto di attacco al server per il VID specificato
    // Bypassa il cooldown client azzerando OFF_ATTACK_COOLDOWN
    inline void AttackVID(uint32_t vid) {
        __try {
            uintptr_t playerBase = GameBase::GetPlayerBase();
            uintptr_t netStream = GameBase::GetNetStreamBase();
            if (!playerBase || !netStream) return;

            // Azzera cooldown attacco lato client
            uintptr_t cooldownAddr = playerBase + OFF_ATTACK_COOLDOWN;
            if (!Memory::IsBadPointer((void*)cooldownAddr))
                *(uint32_t*)cooldownAddr = 0;

            // Chiama SendBattleAttackPacket(__thiscall, this=netStream, type=0, vid)
            uintptr_t fn = Memory::ToReal(FN_SEND_ATTACK_PACKET);
            typedef int(__thiscall* AttackFn)(void*, uint8_t, uint32_t);
            ((AttackFn)fn)((void*)netStream, 0, vid);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    namespace detail {
        inline int  s_delayMs = 0;
        inline void AttackCallback(uint32_t vid, uintptr_t inst) {
            AttackVID(vid);
            if (s_delayMs > 0) Sleep(s_delayMs);
        }
    }

    // Attacca tutti i mob nel raggio specificato
    inline void AttackAllNearby(uintptr_t charMgr, float px, float py,
        float radius, int delayMs) {
        detail::s_delayMs = delayMs;
        if (g_config.whAttackMob)
            EntityList::ForEachMobNearby(charMgr, px, py, radius, detail::AttackCallback);
        if (g_config.whAttackMetin)
            EntityList::ForEachMetinNearby(charMgr, px, py, radius, detail::AttackCallback);
    }

} // namespace WaitHack