#include "global.h"
#include "admin_settings.h"

void AdminSettings_Reset(void)
{
    gSaveBlock2Ptr->adminSettings.magic = ADMIN_SETTINGS_MAGIC;
    gSaveBlock2Ptr->adminSettings.version = ADMIN_SETTINGS_VERSION;
    gSaveBlock2Ptr->adminSettings.shinyRate = ADMIN_SHINY_VANILLA;
    gSaveBlock2Ptr->adminSettings.flags = 0;
    gSaveBlock2Ptr->adminSettings.preset = 0;
    gSaveBlock2Ptr->adminSettings.reserved = 0;
}

void AdminSettings_EnsureInitialized(void)
{
    if (gSaveBlock2Ptr->adminSettings.magic != ADMIN_SETTINGS_MAGIC
     || gSaveBlock2Ptr->adminSettings.version != ADMIN_SETTINGS_VERSION
     || gSaveBlock2Ptr->adminSettings.shinyRate >= ADMIN_SHINY_RATE_COUNT)
        AdminSettings_Reset();
}

u32 AdminSettings_GetShinyThreshold(void)
{
    static const u32 sThresholds[ADMIN_SHINY_RATE_COUNT] =
    {
        [ADMIN_SHINY_VANILLA] = 8,
        [ADMIN_SHINY_1_4096] = 16,
        [ADMIN_SHINY_1_2048] = 32,
        [ADMIN_SHINY_1_1024] = 64,
        [ADMIN_SHINY_1_512] = 128,
        [ADMIN_SHINY_1_256] = 256,
        [ADMIN_SHINY_GUARANTEED] = 65536,
    };

    AdminSettings_EnsureInitialized();
    return sThresholds[gSaveBlock2Ptr->adminSettings.shinyRate];
}

bool8 AdminSettings_GetFlag(u16 flag)
{
    AdminSettings_EnsureInitialized();
    return (gSaveBlock2Ptr->adminSettings.flags & flag) != 0;
}

void AdminSettings_SetFlag(u16 flag, bool8 enabled)
{
    AdminSettings_EnsureInitialized();

    if (enabled)
        gSaveBlock2Ptr->adminSettings.flags |= flag;
    else
        gSaveBlock2Ptr->adminSettings.flags &= ~flag;
}

void AdminSettings_ApplyPreset(u8 preset)
{
    AdminSettings_EnsureInitialized();
    gSaveBlock2Ptr->adminSettings.preset = preset;

    switch (preset)
    {
    case 0:
        gSaveBlock2Ptr->adminSettings.flags = 0;
        break;
    case 1:
        gSaveBlock2Ptr->adminSettings.flags =
            ADMIN_FLAG_NUZLOCKE
          | ADMIN_FLAG_FIRST_ENCOUNTER
          | ADMIN_FLAG_DUPES_CLAUSE
          | ADMIN_FLAG_SHINY_CLAUSE
          | ADMIN_FLAG_FAINT_LOCK;
        break;
    case 2:
        gSaveBlock2Ptr->adminSettings.flags =
            ADMIN_FLAG_NUZLOCKE
          | ADMIN_FLAG_FIRST_ENCOUNTER
          | ADMIN_FLAG_DUPES_CLAUSE
          | ADMIN_FLAG_SHINY_CLAUSE
          | ADMIN_FLAG_FAINT_LOCK
          | ADMIN_FLAG_NO_BATTLE_ITEMS
          | ADMIN_FLAG_HARDCORE;
        break;
    default:
        gSaveBlock2Ptr->adminSettings.preset = 3;
        break;
    }

    AdminSettings_EnforceRules();
}

void AdminSettings_EnforceRules(void)
{
    AdminSettings_EnsureInitialized();

    if (AdminSettings_GetFlag(ADMIN_FLAG_HARDCORE))
        gSaveBlock2Ptr->optionsBattleStyle = OPTIONS_BATTLE_STYLE_SET;
}
