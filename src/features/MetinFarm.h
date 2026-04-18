#pragma once
#include <Windows.h>
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include "../core/Memory.h"
#include "../core/GameBase.h"
#include "../core/EntityList.h"
#include "../core/Pointers.h"

namespace MetinFarm {

    inline void Log(const char* fmt, ...) {
        FILE* f = nullptr;
        fopen_s(&f, "C:\\py\\metinfarm_log.txt", "a");
        if (!f) return;
        char buf[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        DWORD now = GetTickCount();
        fprintf(f, "[%u] %s\n", now, buf);
        fclose(f);
    }

    inline bool IsMetinAlive(uintptr_t charMgr, uint32_t vid) {
        return EntityList::FindInstanceByVID(charMgr, vid) != 0;
    }

    inline void ClearTarget(uintptr_t playerBase) {
        if (!playerBase) return;
        if (!Memory::IsBadPointer((void*)(playerBase + OFF_TARGET_VID_WRITE)))
            *(uint32_t*)(playerBase + OFF_TARGET_VID_WRITE) = 0;
        __try {
            uintptr_t fn = Memory::ToReal(0x0050e690);
            typedef void(__thiscall* KeyFn)(void*, int, uint8_t);
            ((KeyFn)fn)((void*)playerBase, 0, 1);
            Sleep(50);
            ((KeyFn)fn)((void*)playerBase, 0, 0);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline void SetTarget(uintptr_t playerBase, uint32_t vid) {
        __try {
            if (!playerBase) return;
            if (!Memory::IsBadPointer((void*)(playerBase + OFF_TARGET_VID_WRITE)))
                *(uint32_t*)(playerBase + OFF_TARGET_VID_WRITE) = vid;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline void MoveToPosition(uintptr_t playerInst, float x, float y) {
        __try {
            float dest[3] = { x, y, 0.0f };
            uintptr_t fn = Memory::ToReal(FN_MOVE_TO_POSITION);
            typedef int(__thiscall* MoveFn)(void*, float*, float);
            ((MoveFn)fn)((void*)playerInst, dest, 0.0f);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline uint32_t FindNearest(uintptr_t charMgr, float px, float py,
        uint32_t* blacklist = nullptr, int blacklistCount = 0) {
        EntityList::MetinInfo arr[32];
        int count = EntityList::GetSortedMetins(charMgr, px, py, arr, 32);
        if (count == 0) return 0;
        for (int i = 0; i < count; i++) {
            if (arr[i].dist > 5000.0f) continue;
            bool blocked = false;
            if (blacklist) {
                for (int j = 0; j < blacklistCount; j++)
                    if (blacklist[j] == arr[i].vid) { blocked = true; break; }
            }
            if (!blocked) return arr[i].vid;
        }
        return 0;
    }

    inline void OnUpdate(uintptr_t playerBase, uintptr_t charMgr) {
        static uint32_t currentMetinVID = 0;
        static float    lastX = 0, lastY = 0;
        static DWORD    lastMoveTime = 0;
        static DWORD    lastDistCheckTime = 0;
        static float    lastDistToMetin = 999999.0f;
        static uint32_t blacklist[8] = {};
        static int      blacklistCount = 0;
        static bool     isUnblocking = false;
        static DWORD    unblockEndTime = 0;
        static float    lastUnblockAngles[4] = { -1,-1,-1,-1 };
        static int      unblockAttempts = 0;
        static float    unblockPosX[5] = { -1,-1,-1,-1,-1 };
        static float    unblockPosY[5] = { -1,-1,-1,-1,-1 };
        static int      unblockPosIdx = 0;
        static float    destX = 0.0f, destY = 0.0f;
        static DWORD    metinDeadTime = 0;

        uintptr_t netStream = GameBase::GetNetStreamBase();
        uint32_t  mainVID = GameBase::GetMainActorVID(netStream);
        uintptr_t inst = EntityList::FindInstanceByVID(charMgr, mainVID);
        if (!inst) return;

        float px = 0, py = 0;
        GameBase::GetRealPosition(inst, &px, &py);

        float rawX = *(float*)(inst + OFF_INST_POS_X);
        uint32_t rawYbits = *(uint32_t*)(inst + OFF_INST_POS_Y);
        rawYbits ^= 0x80000000;
        float rawY = *reinterpret_cast<float*>(&rawYbits);

        DWORD now = GetTickCount();

        if (isUnblocking) {
            ClearTarget(playerBase);
            MoveToPosition(inst, destX, destY);
            if (now < unblockEndTime) return;
            isUnblocking = false;
            lastMoveTime = now;
            lastX = px; lastY = py;
        }

        float moved = sqrtf((px - lastX) * (px - lastX) + (py - lastY) * (py - lastY));
        if (moved >= 20.0f) {
            lastX = px; lastY = py;
            lastMoveTime = now;
            unblockAttempts = 0;
        }

        if (currentMetinVID == 0 || !IsMetinAlive(charMgr, currentMetinVID)) {
            if (currentMetinVID != 0 && metinDeadTime == 0) {
                metinDeadTime = now;
                ClearTarget(playerBase);
                currentMetinVID = 0;
                return;
            }
            if (metinDeadTime != 0 && now - metinDeadTime < 500) return;
            metinDeadTime = 0;

            currentMetinVID = FindNearest(charMgr, px, py, blacklist, blacklistCount);
            lastMoveTime = now;
            lastDistCheckTime = now;
            lastDistToMetin = 999999.0f;
            lastX = px; lastY = py;
            unblockAttempts = 0;
            if (currentMetinVID == 0) {
                blacklistCount = 0;
                return;
            }
        }

        float distToMetin = 999999.0f;
        uintptr_t metinInst = EntityList::FindInstanceByVID(charMgr, currentMetinVID);
        if (metinInst) {
            float mx = 0, my = 0;
            GameBase::GetRealPosition(metinInst, &mx, &my);
            distToMetin = sqrtf((mx - px) * (mx - px) + (my - py) * (my - py));
        }

        const float ATTACK_RANGE = 5.0f;
        if (distToMetin < ATTACK_RANGE) {
            lastMoveTime = now;
            SetTarget(playerBase, currentMetinVID);
            return;
        }

        if (now - lastDistCheckTime > 10000) {
            if (lastDistToMetin - distToMetin < 100.0f) {
                Log("BLACKLIST: metin %u irraggiungibile", currentMetinVID);
                if (blacklistCount < 8)
                    blacklist[blacklistCount++] = currentMetinVID;
                currentMetinVID = 0;
                lastMoveTime = now;
                return;
            }
            lastDistToMetin = distToMetin;
            lastDistCheckTime = now;
        }

        if (now - lastMoveTime > 8000) {
            Log("BLOCCO: pos=%.0f/%.0f dist=%.0f VID=%u tentativo=%d", px, py, distToMetin, currentMetinVID, unblockAttempts);

            ClearTarget(playerBase);

            float angle = 0.0f;
            int maxTries = 30;
            do {
                angle = (float)(rand() % 360);
                bool angleTooClose = false;
                for (int i = 0; i < 4; i++) {
                    if (lastUnblockAngles[i] < 0) continue;
                    float diff = fabsf(angle - lastUnblockAngles[i]);
                    if (diff > 180.0f) diff = 360.0f - diff;
                    if (diff < 60.0f) { angleTooClose = true; break; }
                }
                if (angleTooClose) continue;
                float rad = angle * 3.14159f / 180.0f;
                float moveDist = 3000.0f + (float)(rand() % 5000);
                destX = rawX + cosf(rad) * moveDist;
                destY = rawY + sinf(rad) * moveDist;
                bool posTooClose = false;
                for (int i = 0; i < 5; i++) {
                    if (unblockPosX[i] < 0) continue;
                    float dx = destX - unblockPosX[i];
                    float dy = destY - unblockPosY[i];
                    if (sqrtf(dx * dx + dy * dy) < 3000.0f) { posTooClose = true; break; }
                }
                if (!posTooClose) break;
            } while (--maxTries > 0);

            lastUnblockAngles[unblockAttempts % 4] = angle;
            unblockPosX[unblockPosIdx] = destX;
            unblockPosY[unblockPosIdx] = destY;
            unblockPosIdx = (unblockPosIdx + 1) % 5;
            unblockAttempts++;

            if (unblockAttempts >= 8) {
                if (blacklistCount < 8)
                    blacklist[blacklistCount++] = currentMetinVID;
                currentMetinVID = 0;
                unblockAttempts = 0;
                lastMoveTime = now;
                return;
            }

            Log("MOVE: dest=%.0f/%.0f angolo=%.0f", destX, destY, angle);
            MoveToPosition(inst, destX, destY);

            isUnblocking = true;
            unblockEndTime = now + 5000;
            lastMoveTime = now;
            return;
        }

        SetTarget(playerBase, currentMetinVID);
    }

} // namespace MetinFarm