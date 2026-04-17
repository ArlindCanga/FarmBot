#pragma once
#include <Windows.h>
#include <cstdint>
#include "Pointers.h"

namespace Memory {

    inline bool IsBadPointer(void* ptr) {
        if (!ptr) return true;
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0) return true;
        if (mbi.State != MEM_COMMIT) return true;
        if (mbi.Protect & PAGE_NOACCESS) return true;
        if (mbi.Protect & PAGE_GUARD) return true;
        return false;
    }

    inline uintptr_t ModuleBase() {
        return (uintptr_t)GetModuleHandle(NULL);
    }

    inline uintptr_t ToReal(uintptr_t ghidraAddr) {
        return ModuleBase() + (ghidraAddr - GHIDRA_BASE);
    }

    template<typename T>
    inline T Read(uintptr_t addr) {
        if (IsBadPointer((void*)addr)) return T{};
        return *(T*)addr;
    }

    template<typename T>
    inline void Write(uintptr_t addr, T value) {
        if (IsBadPointer((void*)addr)) return;
        *(T*)addr = value;
    }

} // namespace Memory
