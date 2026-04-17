#pragma once

struct BotConfig {
    // WaitHack
    bool  waithack = false;
    int   whDelay  = 10;       // ms tra un attacco e l'altro
    float whRange  = 1500.0f;  // raggio di attacco

    // Bot
    bool botActive = false;
};

// Istanza globale
inline BotConfig g_config;
