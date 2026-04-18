#pragma once
#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

// Parser JSON minimale senza dipendenze esterne
// Legge valori hex (0x...) e interi dal config.json
namespace ConfigLoader {

    // Struttura con tutti i valori caricati
    struct Config {
        // Pointers
        uintptr_t PTR_PLAYER_BASE = 0x0099d1fc;
        uintptr_t PTR_CHAR_MGR = 0x0099d200;
        uintptr_t PTR_NET_STREAM = 0x0099db3c;
        uintptr_t PTR_ITEM_MGR = 0x0099d20c;
        uintptr_t PTR_MINIMAP = 0x0099da04;

        // Offsets player
        uintptr_t OFF_TARGET_VID_READ = 0x16a68;
        uintptr_t OFF_TARGET_VID_WRITE = 0x9c;
        uintptr_t OFF_AUTOFARM_FLAG = 0x2c;
        uintptr_t OFF_ATTACK_COOLDOWN = 0x16a68;
        uintptr_t OFF_ATTACK_KEY = 0xb0;

        // Offsets netstream
        uintptr_t OFF_MAIN_ACTOR_VID = 0x98;

        // Offsets instance
        uintptr_t OFF_INST_RACE = 0x184;
        uintptr_t OFF_INST_POS_X = 0x680;
        uintptr_t OFF_INST_POS_Y = 0x684;
        uintptr_t OFF_INST_DEST_POS = 0x524;  // float[3] destinazione movimento
        uintptr_t OFF_INST_MOVING = 0x1a4;  // byte flag "sto muovendo"

        // Offsets charmap
        uintptr_t OFF_CHARMAP_BASE = 0x20;
        uintptr_t OFF_NODE_LEFT = 0x00;
        uintptr_t OFF_NODE_PARENT = 0x04;
        uintptr_t OFF_NODE_RIGHT = 0x08;
        uintptr_t OFF_NODE_KEY = 0x10;
        uintptr_t OFF_NODE_VALUE = 0x14;

        // Race ranges
        uint32_t RACE_MOB_MIN = 100;
        uint32_t RACE_MOB_MAX = 7999;
        uint32_t RACE_METIN_MIN = 8000;
        uint32_t RACE_METIN_MAX = 9999;

        // Functions
        uintptr_t FN_SEND_ATTACK_PACKET = 0x00593630;
        uintptr_t FN_CALL_AUTO_HUNT = 0x00583D70;
        uintptr_t FN_MOVE_TO_POSITION = 0x004ae8f0;  // __thiscall(inst, float* dest, float angle)
        uintptr_t FN_MOVE_TO_POSITION_V2 = 0x004aea30;  // __cdecl(float* dest) — wrapper
        uintptr_t FN_SELECT_ITEM = 0x00650920;

        // Items
        uint32_t BRAVERY_CAPE_VNUM = 70038;
    };

    // Istanza globale
    inline Config cfg;

    // Legge un file in un buffer
    inline std::string ReadFile(const char* path) {
        FILE* f = nullptr;
        fopen_s(&f, path, "r");
        if (!f) return "";
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        std::string buf(size, '\0');
        fread(&buf[0], 1, size, f);
        fclose(f);
        return buf;
    }

    // Estrae il valore stringa per una chiave JSON
    // Supporta "key": "value" e "key": number
    inline std::string GetJsonValue(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\"";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return "";

        pos = json.find(':', pos);
        if (pos == std::string::npos) return "";
        pos++;

        // Salta spazi
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r'))
            pos++;

        if (pos >= json.size()) return "";

        if (json[pos] == '"') {
            // Valore stringa
            pos++;
            size_t end = json.find('"', pos);
            if (end == std::string::npos) return "";
            return json.substr(pos, end - pos);
        }
        else {
            // Valore numerico
            size_t end = pos;
            while (end < json.size() && json[end] != ',' && json[end] != '\n' && json[end] != '}')
                end++;
            std::string val = json.substr(pos, end - pos);
            // Rimuovi spazi
            while (!val.empty() && (val.back() == ' ' || val.back() == '\r' || val.back() == '\n'))
                val.pop_back();
            return val;
        }
    }

    // Converte stringa hex o decimale in uintptr_t
    inline uintptr_t ParseHex(const std::string& s) {
        if (s.empty()) return 0;
        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
            return (uintptr_t)strtoull(s.c_str(), nullptr, 16);
        return (uintptr_t)strtoull(s.c_str(), nullptr, 10);
    }

    // Carica config.json dalla stessa cartella della DLL
    inline bool Load(const char* path = nullptr) {
        char dllPath[MAX_PATH] = {};

        if (path) {
            strcpy_s(dllPath, path);
        }
        else {
            // Trova la cartella della DLL
            HMODULE hMod = nullptr;
            GetModuleHandleExA(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (LPCSTR)&Load, &hMod);
            GetModuleFileNameA(hMod, dllPath, MAX_PATH);
            // Sostituisci nome DLL con config.json
            char* lastSlash = strrchr(dllPath, '\\');
            if (lastSlash) strcpy_s(lastSlash + 1, MAX_PATH - (lastSlash - dllPath + 1), "config.json");
        }

        std::string json = ReadFile(dllPath);
        if (json.empty()) {
            // config.json non trovato — usa valori default
            return false;
        }

        // Pointers
        auto r = [&](const std::string& k) { return ParseHex(GetJsonValue(json, k)); };
        auto ri = [&](const std::string& k) { return (uint32_t)ParseHex(GetJsonValue(json, k)); };

        if (auto v = r("player_base"))      cfg.PTR_PLAYER_BASE = v;
        if (auto v = r("char_mgr"))         cfg.PTR_CHAR_MGR = v;
        if (auto v = r("net_stream"))       cfg.PTR_NET_STREAM = v;
        if (auto v = r("item_mgr"))         cfg.PTR_ITEM_MGR = v;
        if (auto v = r("minimap"))          cfg.PTR_MINIMAP = v;

        // Offsets player
        if (auto v = r("target_vid_read"))  cfg.OFF_TARGET_VID_READ = v;
        if (auto v = r("target_vid_write")) cfg.OFF_TARGET_VID_WRITE = v;
        if (auto v = r("autofarm_flag"))    cfg.OFF_AUTOFARM_FLAG = v;
        if (auto v = r("attack_cooldown"))  cfg.OFF_ATTACK_COOLDOWN = v;
        if (auto v = r("attack_key"))       cfg.OFF_ATTACK_KEY = v;

        // Offsets netstream
        if (auto v = r("main_actor_vid"))   cfg.OFF_MAIN_ACTOR_VID = v;

        // Offsets instance
        if (auto v = r("race"))             cfg.OFF_INST_RACE = v;
        if (auto v = r("pos_x"))            cfg.OFF_INST_POS_X = v;
        if (auto v = r("pos_y"))            cfg.OFF_INST_POS_Y = v;
        if (auto v = r("dest_pos"))         cfg.OFF_INST_DEST_POS = v;
        if (auto v = r("moving"))           cfg.OFF_INST_MOVING = v;

        // Offsets charmap
        if (auto v = r("base"))             cfg.OFF_CHARMAP_BASE = v;
        if (auto v = r("node_left"))        cfg.OFF_NODE_LEFT = v;
        if (auto v = r("node_parent"))      cfg.OFF_NODE_PARENT = v;
        if (auto v = r("node_right"))       cfg.OFF_NODE_RIGHT = v;
        if (auto v = r("node_key"))         cfg.OFF_NODE_KEY = v;
        if (auto v = r("node_value"))       cfg.OFF_NODE_VALUE = v;

        // Race ranges
        if (auto v = ri("mob_min"))         cfg.RACE_MOB_MIN = v;
        if (auto v = ri("mob_max"))         cfg.RACE_MOB_MAX = v;
        if (auto v = ri("metin_min"))       cfg.RACE_METIN_MIN = v;

        // Functions
        if (auto v = r("send_attack_packet"))  cfg.FN_SEND_ATTACK_PACKET = v;
        if (auto v = r("call_auto_hunt"))      cfg.FN_CALL_AUTO_HUNT = v;
        if (auto v = r("move_to_position"))    cfg.FN_MOVE_TO_POSITION = v;
        if (auto v = r("move_to_position_v2")) cfg.FN_MOVE_TO_POSITION_V2 = v;
        if (auto v = r("select_item"))         cfg.FN_SELECT_ITEM = v;

        // Items
        if (auto v = ri("bravery_cape_vnum")) cfg.BRAVERY_CAPE_VNUM = v;

        return true;
    }

} // namespace ConfigLoader