# FarmBot — Metin2 Private Server DLL Injection

> ⚠️ **Educational purposes only.** This project was developed on a private/retro Metin2 server for learning and research. It is not intended for use on official servers or any environment where it would violate terms of service. The author takes no responsibility for misuse.

---

## Overview

FarmBot is a DLL injection project targeting a private Metin2 server client (32-bit PE binary). The goal was to explore reverse engineering techniques on a real-world game client — analyzing memory structures, locating functions, and understanding how the client communicates with the server.

---

## What We Did

### 1. Static Analysis with Ghidra
- Loaded the 32-bit `metin2client.exe` into Ghidra
- Located singleton pointers for core classes: `CPythonPlayer`, `CPythonCharacterManager`, `CPythonNetworkStream`
- Identified key functions by cross-referencing debug strings left in the binary (e.g. `"Send Battle Attack Packet Error"`, `"playerSetAttackKeyState"`)
- Analyzed the embedded Python module table (`PyMethodDef`) to map Python-callable functions to their C counterparts
- Reconstructed key data structures: `CInstanceBase` (entity position, race), the character manager's `std::map` red-black tree

### 2. Dynamic Analysis with x32dbg
- Used hardware and software breakpoints to trace execution at runtime
- Identified calling conventions (`__thiscall`, `__cdecl`, `__fastcall`) for key functions
- Verified memory offsets for player position, target VID, attack cooldown timer

### 3. Python Bytecode Extraction
- The client embeds `python27.dll` and compiles UI scripts into `.pyc` files packed in proprietary archives
- Extracted bytecode at runtime using `marshal.dump` on live `func_code` objects
- Analyzed module structure and available functions through the embedded Python interpreter

### 4. Network Packet Analysis
- Identified `SendBattleAttackPacket` as a `__thiscall` method on `CPythonNetworkStream`
- Discovered that bypassing the client-side attack cooldown (offset `0x16a68` on `CPythonPlayer`) allows sending attack packets directly to the server
- Analyzed the entity list structure to enumerate nearby entities by race type and distance

### 5. DLL Injection
- Injected a custom DLL into the game process at startup
- Implemented a Win32 dark-theme GUI with configurable range and delay sliders
- Bot loop runs on a background thread reading entity list and dispatching attacks

---

## Features (v1)

- **WaitHack** — sends attack packets directly to the server for all nearby mobs, bypassing the client-side cooldown. Attack range and delay between packets are configurable at runtime via GUI sliders.
- **Config-driven** — all memory addresses and offsets are stored in `config.json`, making the tool portable across different private server builds without recompilation.

### Planned (v2)
- Automatic mob farm
- Automatic metin farm

---

## Architecture

```
FarmBot/
├── config.json                  <- Memory addresses & offsets (server-specific)
└── src/
    ├── Config.h                 <- Runtime bot configuration struct
    ├── dllmain.cpp              <- DLL entry point & bot loop
    ├── core/
    │   ├── ConfigLoader.h       <- JSON parser, loads config.json at runtime
    │   ├── Pointers.h           <- Macro aliases over ConfigLoader::cfg
    │   ├── Memory.h             <- Safe read/write, IsBadPointer, ToReal
    │   ├── GameBase.h           <- Singleton access, player/entity data
    │   └── EntityList.h         <- std::map red-black tree traversal
    ├── features/
    │   └── WaitHack.h           <- Direct attack packet injection
    └── gui/
        └── GUI.h                <- Win32 dark-theme GUI
```

---

## Build

- **Visual Studio 2022** with CMake support
- Platform: **x86 (Win32)** — the game client is a 32-bit binary
- Output: `FarmBot.dll` — injected via any standard DLL injector

---

## Techniques & Concepts

| Topic | Applied |
|---|---|
| Reverse engineering | Ghidra decompilation, calling convention identification |
| Memory analysis | Pointer chasing, struct offset mapping, red-black tree traversal |
| x86 internals | `__thiscall`, vtable calls, SEH (`__try/__except`) |
| Python internals | Bytecode extraction, `PyMethodDef` table analysis, embedded interpreter interop |
| Win32 | DLL injection, dark-theme GUI, multi-threading |
| Network | Game packet structure analysis, direct packet injection |

---

## Disclaimer

This project targets a **private/retro Metin2 server** and was built purely for **educational and research purposes** — to study reverse engineering, memory analysis, and low-level Windows programming on a real binary. No official servers were involved. The code is shared as a reference for learning, not as a ready-to-use cheating tool.
