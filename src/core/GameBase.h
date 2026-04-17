#pragma once
#include <cstdint>
#include "Memory.h"
#include "Pointers.h"

namespace GameBase {

    inline uintptr_t GetPlayerBase() {
        uintptr_t a = Memory::ToReal(PTR_PLAYER_BASE);
        if (Memory::IsBadPointer((void*)a)) return 0;
        return *(uintptr_t*)a;
    }

    inline uintptr_t GetCharMgrBase() {
        uintptr_t a = Memory::ToReal(PTR_CHAR_MGR);
        if (Memory::IsBadPointer((void*)a)) return 0;
        return *(uintptr_t*)a;
    }

    inline uintptr_t GetNetStreamBase() {
        uintptr_t a = Memory::ToReal(PTR_NET_STREAM);
        if (Memory::IsBadPointer((void*)a)) return 0;
        return *(uintptr_t*)a;
    }

    inline uint32_t GetMainActorVID(uintptr_t netStream) {
        if (!netStream || Memory::IsBadPointer((void*)(netStream + OFF_MAIN_ACTOR_VID))) return 0;
        return *(uint32_t*)(netStream + OFF_MAIN_ACTOR_VID);
    }

    inline uint8_t GetAutoFarmFlag(uintptr_t playerBase) {
        if (!playerBase || Memory::IsBadPointer((void*)(playerBase + OFF_AUTOFARM_FLAG))) return 0;
        return *(uint8_t*)(playerBase + OFF_AUTOFARM_FLAG);
    }

    inline uint32_t GetTargetVID(uintptr_t playerBase) {
        if (!playerBase || Memory::IsBadPointer((void*)(playerBase + OFF_TARGET_VID_READ))) return 0;
        return *(uint32_t*)(playerBase + OFF_TARGET_VID_READ);
    }

    inline void SetTargetVID(uintptr_t playerBase, uint32_t vid) {
        if (!playerBase || Memory::IsBadPointer((void*)(playerBase + OFF_TARGET_VID_WRITE))) return;
        *(uint32_t*)(playerBase + OFF_TARGET_VID_WRITE) = vid;
    }

    inline void GetRealPosition(uintptr_t inst, float* x, float* y) {
        __try {
            if (!inst || Memory::IsBadPointer((void*)inst)) return;
            *x = *(float*)(inst + OFF_INST_POS_X) / 100.0f;
            uint32_t rawY = *(uint32_t*)(inst + OFF_INST_POS_Y) ^ 0x80000000;
            *y = *(float*)&rawY / 100.0f;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline uint32_t GetEntityRace(uintptr_t inst) {
        __try {
            if (!inst || Memory::IsBadPointer((void*)(inst + OFF_INST_RACE))) return 0;
            return *(uint32_t*)(inst + OFF_INST_RACE);
        } __except(EXCEPTION_EXECUTE_HANDLER) { return 0; }
    }

} // namespace GameBase
