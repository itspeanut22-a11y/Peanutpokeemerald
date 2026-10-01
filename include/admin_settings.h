#ifndef GUARD_ADMIN_SETTINGS_H
#define GUARD_ADMIN_SETTINGS_H

#include "gba/types.h"

#define ADMIN_SETTINGS_MAGIC 0xAD4D
#define ADMIN_SETTINGS_VERSION 1

enum AdminShinyRate
{
    ADMIN_SHINY_VANILLA,
    ADMIN_SHINY_1_4096,
    ADMIN_SHINY_1_2048,
    ADMIN_SHINY_1_1024,
    ADMIN_SHINY_1_512,
    ADMIN_SHINY_1_256,
    ADMIN_SHINY_GUARANTEED,
    ADMIN_SHINY_RATE_COUNT
};

enum AdminSettingFlags
{
    ADMIN_FLAG_NUZLOCKE        = (1 << 0),
    ADMIN_FLAG_FIRST_ENCOUNTER = (1 << 1),
    ADMIN_FLAG_DUPES_CLAUSE    = (1 << 2),
    ADMIN_FLAG_SHINY_CLAUSE    = (1 << 3),
    ADMIN_FLAG_FAINT_LOCK      = (1 << 4),
    ADMIN_FLAG_NO_BATTLE_ITEMS = (1 << 5),
    ADMIN_FLAG_HARDCORE        = (1 << 6),
};

struct AdminSettings
{
    u16 magic;
    u8 version;
    u8 shinyRate;
    u16 flags;
    u8 preset;
    u8 reserved;
};

void AdminSettings_EnsureInitialized(void);
void AdminSettings_Reset(void);
u32 AdminSettings_GetShinyThreshold(void);
bool8 AdminSettings_GetFlag(u16 flag);
void AdminSettings_SetFlag(u16 flag, bool8 enabled);
void AdminSettings_ApplyPreset(u8 preset);
void AdminSettings_EnforceRules(void);

#endif
