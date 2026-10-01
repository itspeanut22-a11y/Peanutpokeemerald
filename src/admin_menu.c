#include "global.h"
#include "admin_menu.h"
#include "admin_settings.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

static void AdminMenu_MainCB(void);
static void AdminMenu_VBlankCB(void);
static void AdminMenu_InitBgs(void);
static void AdminMenu_Print(void);
static void Task_AdminMenuInput(u8 taskId);

static const u8 sText_Title[] = _("ADMIN SETTINGS");
static const u8 sText_ShinyRate[] = _("Shiny Rate");
static const u8 sText_Nuzlocke[] = _("Nuzlocke");
static const u8 sText_Hardcore[] = _("Hardcore");
static const u8 sText_FaintLock[] = _("Faint Lock");
static const u8 sText_NoItems[] = _("No Battle Items");
static const u8 sText_Exit[] = _("Exit");

static const u8 sText_Off[] = _("OFF");
static const u8 sText_On[] = _("ON");

static const struct BgTemplate sAdminBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    }
};

static const struct WindowTemplate sAdminWindowTemplates[] =
{
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 26,
        .height = 18,
        .paletteNum = 15,
        .baseBlock = 1
    },
    DUMMY_WIN_TEMPLATE
};

void CB2_InitAdminMenu(void)
{
    ResetTasks();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetPaletteFade();
    ScanlineEffect_Stop();

    AdminSettings_EnsureInitialized();
    AdminMenu_InitBgs();

    SetVBlankCallback(AdminMenu_VBlankCB);
    SetMainCallback2(AdminMenu_MainCB);

    CreateTask(Task_AdminMenuInput, 0);
    AdminMenu_Print();
}

static void AdminMenu_InitBgs(void)
{
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sAdminBgTemplates, ARRAY_COUNT(sAdminBgTemplates));
    InitWindows(sAdminWindowTemplates);
    DeactivateAllTextPrinters();

    FillWindowPixelBuffer(0, PIXEL_FILL(1));
    PutWindowTilemap(0);
    CopyWindowToVram(0, COPYWIN_FULL);

    ShowBg(0);
}

static void AdminMenu_Print(void)
{
    u8 color[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

    FillWindowPixelBuffer(0, PIXEL_FILL(1));

    AddTextPrinterParameterized3(0, FONT_NORMAL, 6, 4, color, 0, sText_Title);

    AddTextPrinterParameterized3(0, FONT_NORMAL, 6, 28, color, 0, sText_ShinyRate);
    AddTextPrinterParameterized3(0, FONT_NORMAL, 6, 48, color, 0, sText_Nuzlocke);
    AddTextPrinterParameterized3(0, FONT_NORMAL, 6, 68, color, 0, sText_Hardcore);
    AddTextPrinterParameterized3(0, FONT_NORMAL, 6, 88, color, 0, sText_FaintLock);
    AddTextPrinterParameterized3(0, FONT_NORMAL, 6, 108, color, 0, sText_NoItems);
    AddTextPrinterParameterized3(0, FONT_NORMAL, 6, 128, color, 0, sText_Exit);

    AddTextPrinterParameterized3(
        0,
        FONT_NORMAL,
        160,
        48,
        color,
        0,
        AdminSettings_GetFlag(ADMIN_FLAG_NUZLOCKE) ? sText_On : sText_Off);

    AddTextPrinterParameterized3(
        0,
        FONT_NORMAL,
        160,
        68,
        color,
        0,
        AdminSettings_GetFlag(ADMIN_FLAG_HARDCORE) ? sText_On : sText_Off);

    AddTextPrinterParameterized3(
        0,
        FONT_NORMAL,
        160,
        88,
        color,
        0,
        AdminSettings_GetFlag(ADMIN_FLAG_FAINT_LOCK) ? sText_On : sText_Off);

    AddTextPrinterParameterized3(
        0,
        FONT_NORMAL,
        160,
        108,
        color,
        0,
        AdminSettings_GetFlag(ADMIN_FLAG_NO_BATTLE_ITEMS) ? sText_On : sText_Off);

    CopyWindowToVram(0, COPYWIN_GFX);
}

static void Task_AdminMenuInput(u8 taskId)
{
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        DestroyTask(taskId);
        SetMainCallback2(CB2_ReturnToField);
    }
}

static void AdminMenu_MainCB(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void AdminMenu_VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}
