#pragma once

struct BotConfig {
    bool  waithack = false;
    bool  whAttackMob = true;
    bool  whAttackMetin = false;
    int   whDelay = 10;
    float whRange = 1500.0f;
    bool  botActive = false;
    bool  metinFarm = false;
};

inline BotConfig g_config;