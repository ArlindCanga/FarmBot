#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
#include "Memory.h"
#include "GameBase.h"
#include "Pointers.h"

namespace EntityList {

    inline uintptr_t GetLeftmost(uintptr_t node, uintptr_t header) {
        uintptr_t cur = node;
        while (true) {
            if (!cur || cur == header || Memory::IsBadPointer((void*)cur)) break;
            uintptr_t left = *(uintptr_t*)(cur + OFF_NODE_LEFT);
            if (!left || left == header || Memory::IsBadPointer((void*)left)) break;
            cur = left;
        }
        return cur;
    }

    inline uintptr_t GetNextNode(uintptr_t node, uintptr_t header) {
        __try {
            uintptr_t right = *(uintptr_t*)(node + OFF_NODE_RIGHT);
            if (right && right != header && !Memory::IsBadPointer((void*)right))
                return GetLeftmost(right, header);
            uintptr_t parent = *(uintptr_t*)(node + OFF_NODE_PARENT);
            while (parent != header && node == *(uintptr_t*)(parent + OFF_NODE_RIGHT)) {
                node = parent;
                parent = *(uintptr_t*)(parent + OFF_NODE_PARENT);
            }
            return parent;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { return header; }
    }

    inline uintptr_t FindInstanceByVID(uintptr_t charMgr, uint32_t vid) {
        uintptr_t mapBase = charMgr + OFF_CHARMAP_BASE;
        uintptr_t header = *(uintptr_t*)(mapBase);
        uintptr_t size = *(uintptr_t*)(mapBase + 0x04);
        if (!size || size > 10000) return 0;
        uintptr_t root = *(uintptr_t*)(header + 0x04);
        if (root == header || Memory::IsBadPointer((void*)root)) return 0;
        uintptr_t node = GetLeftmost(root, header);
        uint32_t count = 0;
        while (node != header && !Memory::IsBadPointer((void*)node) && count < size) {
            __try {
                if (*(uint32_t*)(node + OFF_NODE_KEY) == vid)
                    return *(uintptr_t*)(node + OFF_NODE_VALUE);
                count++;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) { break; }
            node = GetNextNode(node, header);
        }
        return 0;
    }

    // Itera tutti i mob nel raggio specificato
    inline void ForEachMobNearby(uintptr_t charMgr, float px, float py, float radius,
        void(*callback)(uint32_t, uintptr_t)) {
        uintptr_t mapBase = charMgr + OFF_CHARMAP_BASE;
        uintptr_t header = *(uintptr_t*)(mapBase);
        uintptr_t size = *(uintptr_t*)(mapBase + 0x04);
        if (!size || size > 10000) return;
        uintptr_t root = *(uintptr_t*)(header + 0x04);
        if (root == header || Memory::IsBadPointer((void*)root)) return;

        uintptr_t netStream = GameBase::GetNetStreamBase();
        uint32_t  myVID = GameBase::GetMainActorVID(netStream);

        uintptr_t node = GetLeftmost(root, header);
        uint32_t  count = 0;
        while (node != header && !Memory::IsBadPointer((void*)node) && count < size) {
            __try {
                uint32_t  vid = *(uint32_t*)(node + OFF_NODE_KEY);
                uintptr_t inst = *(uintptr_t*)(node + OFF_NODE_VALUE);
                if (vid != myVID && vid != 0) {
                    uint32_t race = GameBase::GetEntityRace(inst);
                    if (race >= RACE_MOB_MIN && race < RACE_MOB_MAX) {
                        float ex = 0, ey = 0;
                        GameBase::GetRealPosition(inst, &ex, &ey);
                        float d = sqrtf((ex - px) * (ex - px) + (ey - py) * (ey - py));
                        if (d <= radius) callback(vid, inst);
                    }
                }
                count++;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) { break; }
            node = GetNextNode(node, header);
        }
    }

    inline void ForEachMetinNearby(uintptr_t charMgr, float px, float py, float radius,
        void(*callback)(uint32_t, uintptr_t)) {
        uintptr_t mapBase = charMgr + OFF_CHARMAP_BASE;
        uintptr_t header = *(uintptr_t*)(mapBase);
        uintptr_t size = *(uintptr_t*)(mapBase + 0x04);
        if (!size || size > 10000) return;
        uintptr_t root = *(uintptr_t*)(header + 0x04);
        if (root == header || Memory::IsBadPointer((void*)root)) return;

        uintptr_t node = GetLeftmost(root, header);
        uint32_t  count = 0;
        while (node != header && !Memory::IsBadPointer((void*)node) && count < size) {
            __try {
                uint32_t  vid = *(uint32_t*)(node + OFF_NODE_KEY);
                uintptr_t inst = *(uintptr_t*)(node + OFF_NODE_VALUE);
                if (vid != 0) {
                    uint32_t race = GameBase::GetEntityRace(inst);
                    if (race >= RACE_METIN_MIN && race <= RACE_METIN_MAX) {
                        float ex = 0, ey = 0;
                        GameBase::GetRealPosition(inst, &ex, &ey);
                        float d = sqrtf((ex - px) * (ex - px) + (ey - py) * (ey - py));
                        if (d <= radius) callback(vid, inst);
                    }
                }
                count++;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) { break; }
            node = GetNextNode(node, header);
        }
    }

    // Trova la metin più vicina
    inline uint32_t FindNearestMetin(uintptr_t charMgr, float px, float py,
        float* outX = nullptr, float* outY = nullptr) {
        uintptr_t mapBase = charMgr + OFF_CHARMAP_BASE;
        uintptr_t header = *(uintptr_t*)(mapBase);
        uintptr_t size = *(uintptr_t*)(mapBase + 0x04);
        if (!size || size > 10000) return 0;
        uintptr_t root = *(uintptr_t*)(header + 0x04);
        if (root == header || Memory::IsBadPointer((void*)root)) return 0;

        uint32_t  nearVID = 0;
        float     nearDist = 999999.0f;
        uintptr_t node = GetLeftmost(root, header);
        uint32_t  count = 0;
        while (node != header && !Memory::IsBadPointer((void*)node) && count < size) {
            __try {
                uint32_t  vid = *(uint32_t*)(node + OFF_NODE_KEY);
                uintptr_t inst = *(uintptr_t*)(node + OFF_NODE_VALUE);
                if (GameBase::GetEntityRace(inst) >= RACE_METIN_MIN) {
                    float ex = 0, ey = 0;
                    GameBase::GetRealPosition(inst, &ex, &ey);
                    float d = sqrtf((ex - px) * (ex - px) + (ey - py) * (ey - py));
                    if (d < nearDist) {
                        nearDist = d;
                        nearVID = vid;
                        if (outX) *outX = ex;
                        if (outY) *outY = ey;
                    }
                }
                count++;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) { break; }
            node = GetNextNode(node, header);
        }
        return nearVID;
    }

    struct MetinInfo {
        uint32_t  vid;
        float     x;
        float     y;
        float     dist;
    };

    namespace detail {
        inline void CollectMetin(uintptr_t inst, uint32_t vid, float px, float py,
            MetinInfo* arr, int* count, int maxCount) {
            __try {
                if (GameBase::GetEntityRace(inst) >= RACE_METIN_MIN &&
                    GameBase::GetEntityRace(inst) <= RACE_METIN_MAX) {
                    float ex = 0, ey = 0;
                    GameBase::GetRealPosition(inst, &ex, &ey);
                    float d = sqrtf((ex - px) * (ex - px) + (ey - py) * (ey - py));
                    if (*count < maxCount) {
                        arr[*count] = { vid, ex, ey, d };
                        (*count)++;
                    }
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
        }
    }

    inline int GetSortedMetins(uintptr_t charMgr, float px, float py,
        MetinInfo* outArr, int maxCount) {
        int count = 0;
        uintptr_t mapBase = charMgr + OFF_CHARMAP_BASE;
        uintptr_t header = *(uintptr_t*)(mapBase);
        uintptr_t size = *(uintptr_t*)(mapBase + 0x04);
        if (!size || size > 10000) return 0;
        uintptr_t root = *(uintptr_t*)(header + 0x04);
        if (root == header || Memory::IsBadPointer((void*)root)) return 0;

        uintptr_t node = GetLeftmost(root, header);
        uint32_t  cnt = 0;
        while (node != header && !Memory::IsBadPointer((void*)node) && cnt < size) {
            __try {
                uint32_t  vid = *(uint32_t*)(node + OFF_NODE_KEY);
                uintptr_t inst = *(uintptr_t*)(node + OFF_NODE_VALUE);
                detail::CollectMetin(inst, vid, px, py, outArr, &count, maxCount);
                cnt++;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) { break; }
            node = GetNextNode(node, header);
        }

        // Bubble sort per distanza (max 32 metin, va bene)
        for (int i = 0; i < count - 1; i++)
            for (int j = 0; j < count - i - 1; j++)
                if (outArr[j].dist > outArr[j + 1].dist)
                    std::swap(outArr[j], outArr[j + 1]);

        return count;
    }

}