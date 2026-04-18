#pragma once
#include <cstdint>
#include "ConfigLoader.h"

// Base Ghidra
constexpr uintptr_t GHIDRA_BASE = 0x00400000;

// Tutti i valori vengono letti da config.json tramite ConfigLoader::cfg
#define PTR_PLAYER_BASE             ConfigLoader::cfg.PTR_PLAYER_BASE
#define PTR_CHAR_MGR                ConfigLoader::cfg.PTR_CHAR_MGR
#define PTR_NET_STREAM              ConfigLoader::cfg.PTR_NET_STREAM
#define PTR_ITEM_MGR                ConfigLoader::cfg.PTR_ITEM_MGR
#define PTR_MINIMAP                 ConfigLoader::cfg.PTR_MINIMAP

#define OFF_TARGET_VID_READ         ConfigLoader::cfg.OFF_TARGET_VID_READ
#define OFF_TARGET_VID_WRITE        ConfigLoader::cfg.OFF_TARGET_VID_WRITE
#define OFF_AUTOFARM_FLAG           ConfigLoader::cfg.OFF_AUTOFARM_FLAG
#define OFF_ATTACK_COOLDOWN         ConfigLoader::cfg.OFF_ATTACK_COOLDOWN
#define OFF_ATTACK_KEY              ConfigLoader::cfg.OFF_ATTACK_KEY

#define OFF_MAIN_ACTOR_VID          ConfigLoader::cfg.OFF_MAIN_ACTOR_VID

#define OFF_INST_RACE               ConfigLoader::cfg.OFF_INST_RACE
#define OFF_INST_POS_X              ConfigLoader::cfg.OFF_INST_POS_X
#define OFF_INST_POS_Y              ConfigLoader::cfg.OFF_INST_POS_Y

#define OFF_CHARMAP_BASE            ConfigLoader::cfg.OFF_CHARMAP_BASE
#define OFF_NODE_LEFT               ConfigLoader::cfg.OFF_NODE_LEFT
#define OFF_NODE_PARENT             ConfigLoader::cfg.OFF_NODE_PARENT
#define OFF_NODE_RIGHT              ConfigLoader::cfg.OFF_NODE_RIGHT
#define OFF_NODE_KEY                ConfigLoader::cfg.OFF_NODE_KEY
#define OFF_NODE_VALUE              ConfigLoader::cfg.OFF_NODE_VALUE

#define RACE_MOB_MIN                ConfigLoader::cfg.RACE_MOB_MIN
#define RACE_MOB_MAX                ConfigLoader::cfg.RACE_MOB_MAX
#define RACE_METIN_MIN              ConfigLoader::cfg.RACE_METIN_MIN
#define RACE_METIN_MAX              ConfigLoader::cfg.RACE_METIN_MAX

#define FN_SEND_ATTACK_PACKET       ConfigLoader::cfg.FN_SEND_ATTACK_PACKET
#define FN_CALL_AUTO_HUNT           ConfigLoader::cfg.FN_CALL_AUTO_HUNT
#define FN_MOVE_TO_POSITION         ConfigLoader::cfg.FN_MOVE_TO_POSITION
#define FN_SELECT_ITEM              ConfigLoader::cfg.FN_SELECT_ITEM

#define BRAVERY_CAPE_VNUM           ConfigLoader::cfg.BRAVERY_CAPE_VNUM
