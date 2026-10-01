#include "global.h"
#include "admin_menu.h"
#include "admin_settings.h"
#include "bg.h"
#include "gpu_regs.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sprite.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/rgb.h"

#define tSelection data[0]

enum
{
    ITEM_SHINY,
    ITEM_NUZLOCKE,
    ITEM_HARDCORE,
    ITEM_FAINT_LOCK,
    ITEM_NO_ITEMS,
    ITEM_EXIT,
    ITEM_COUNT
};

enum
{
    WIN_HEADER,
    WIN_ADMIN
};

static void MainCB(void);
static void VBlankCB(void);
static void Task_FadeIn(u8 taskId);
static void Task_ProcessInput(u8 taskId);
static void Task_FadeOut(u8 taskId);
static void DrawMenu(void);
static void HighlightItem(u8 selection);

static const u8 sText_Title[] = _("ADMIN SETTINGS");
static const u8 sText_Shiny[] = _("Shiny Rate");
static const u8 sText_Nuzlocke[] = _("Nuzlocke");
static const u8 sText_Hardcore[] = _("Hardcore");
static const u8 sText_FaintLock[] = _("Faint Lock");
static const u8 sText_NoItems[] = _("No Battle Items");
static const u8 sText_Exit[] = _("Exit");
static const u8 sText_On[] = _("ON");
static const u8 sText_Off[] = _("OFF");

static const u8 sText_ShinyVanilla[] = _("1/8192");
static const u8 sText_Shiny4096[] = _("1/4096");
static const u8 sText_Shiny2048[] = _("1/2048");
static const u8 sText_Shiny1024[] = _("1/1024");
static const u8 sText_Shiny512[] = _("1/512");
static const u8 sText_Shiny256[] = _("1/256");
static const u8 sText_ShinyAlways[] = _("ALWAYS");

static const u8 *const sShinyNames[ADMIN_SHINY_RATE_COUNT] =
{
    sText_ShinyVanilla,
    sText_Shiny4096,
    sText_Shiny2048,
    sText_Shiny1024,
    sText_Shiny512,
    sText_Shiny256,
    sText_ShinyAlways
};

static const struct WindowTemplate sWindows[] =
{
    [WIN_HEADER] =
    {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 26,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 2
    },
    [WIN_ADMIN] =
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 5,
        .width = 26,
        .height = 14,
        .paletteNum = 1,
        .baseBlock = 0x36
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sBgs[] =
{
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 0,
        .charBaseIndex = 1,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    }
};

static const u16 sBgPal[] = {RGB(17, 18, 31)};

static void MainCB(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

void CB2_InitAdminMenu(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        gMain.state++;
        break;

    case 1:
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);

        SetGpuReg(REG_OFFSET_DISPCNT, 0);

        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgs, ARRAY_COUNT(sBgs));

        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        ChangeBgX(2, 0, BG_COORD_SET);
        ChangeBgY(2, 0, BG_COORD_SET);
        ChangeBgX(3, 0, BG_COORD_SET);
        ChangeBgY(3, 0, BG_COORD_SET);

        InitWindows(sWindows);
        DeactivateAllTextPrinters();

        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0);
        SetGpuReg(REG_OFFSET_WINOUT,
                  WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_CLR);

        SetGpuReg(REG_OFFSET_BLDCNT,
                  BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_DARKEN);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 4);

        SetGpuReg(REG_OFFSET_DISPCNT,
                  DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);

        ShowBg(0);
        ShowBg(1);
        gMain.state++;
        break;

    case 2:
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        AdminSettings_EnsureInitialized();
        gMain.state++;
        break;

    case 3:
        LoadBgTiles(
            1,
            GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles,
            0x120,
            0x1A2
        );
        gMain.state++;
        break;

    case 4:
        LoadPalette(sBgPal, BG_PLTT_ID(0), sizeof(sBgPal));
        LoadPalette(
            GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal,
            BG_PLTT_ID(7),
            PLTT_SIZE_4BPP
        );
        gMain.state++;
        break;

    case 5:
        PutWindowTilemap(WIN_HEADER);
        FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
        AddTextPrinterParameterized(
            WIN_HEADER, FONT_NORMAL, sText_Title,
            8, 1, TEXT_SKIP_DRAW, NULL
        );
        CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
        gMain.state++;
        break;

    case 6:
    {
        u8 taskId;

        PutWindowTilemap(WIN_ADMIN);
        DrawMenu();

        taskId = CreateTask(Task_FadeIn, 0);
        gTasks[taskId].tSelection = 0;

        HighlightItem(0);

        CopyWindowToVram(WIN_ADMIN, COPYWIN_FULL);

        BeginNormalPaletteFade(
            PALETTES_ALL, 0, 16, 0, RGB_BLACK
        );

        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB);
        break;
    }
    }
}

static void DrawValue(const u8 *text, u8 y)
{
    AddTextPrinterParameterized(
        WIN_ADMIN,
        FONT_NORMAL,
        text,
        144,
        y,
        TEXT_SKIP_DRAW,
        NULL
    );
}

static void DrawMenu(void)
{
    FillWindowPixelBuffer(WIN_ADMIN, PIXEL_FILL(1));

    AddTextPrinterParameterized(
        WIN_ADMIN, FONT_NORMAL,
        sText_Shiny, 8, 1, TEXT_SKIP_DRAW, NULL
    );

    AddTextPrinterParameterized(
        WIN_ADMIN, FONT_NORMAL,
        sText_Nuzlocke, 8, 17, TEXT_SKIP_DRAW, NULL
    );

    AddTextPrinterParameterized(
        WIN_ADMIN, FONT_NORMAL,
        sText_Hardcore, 8, 33, TEXT_SKIP_DRAW, NULL
    );

    AddTextPrinterParameterized(
        WIN_ADMIN, FONT_NORMAL,
        sText_FaintLock, 8, 49, TEXT_SKIP_DRAW, NULL
    );

    AddTextPrinterParameterized(
        WIN_ADMIN, FONT_NORMAL,
        sText_NoItems, 8, 65, TEXT_SKIP_DRAW, NULL
    );

    AddTextPrinterParameterized(
        WIN_ADMIN, FONT_NORMAL,
        sText_Exit, 8, 81, TEXT_SKIP_DRAW, NULL
    );

    DrawValue(
        sShinyNames[gSaveBlock2Ptr->adminSettings.shinyRate],
        1
    );

    DrawValue(
        AdminSettings_GetFlag(ADMIN_FLAG_NUZLOCKE)
            ? sText_On : sText_Off,
        17
    );

    DrawValue(
        AdminSettings_GetFlag(ADMIN_FLAG_HARDCORE)
            ? sText_On : sText_Off,
        33
    );

    DrawValue(
        AdminSettings_GetFlag(ADMIN_FLAG_FAINT_LOCK)
            ? sText_On : sText_Off,
        49
    );

    DrawValue(
        AdminSettings_GetFlag(ADMIN_FLAG_NO_BATTLE_ITEMS)
            ? sText_On : sText_Off,
        65
    );

    CopyWindowToVram(WIN_ADMIN, COPYWIN_GFX);
}

static void HighlightItem(u8 selection)
{
    SetGpuReg(
        REG_OFFSET_WIN0H,
        WIN_RANGE(16, DISPLAY_WIDTH - 16)
    );

    SetGpuReg(
        REG_OFFSET_WIN0V,
        WIN_RANGE(selection * 16 + 40,
                  selection * 16 + 56)
    );
}

static void ToggleFlag(u16 flag)
{
    AdminSettings_SetFlag(
        flag,
        !AdminSettings_GetFlag(flag)
    );

    AdminSettings_EnforceRules();
}

static void ChangeCurrentSetting(s8 direction, u8 selection)
{
    switch (selection)
    {
    case ITEM_SHINY:
        if (direction > 0)
        {
            gSaveBlock2Ptr->adminSettings.shinyRate++;

            if (gSaveBlock2Ptr->adminSettings.shinyRate
                >= ADMIN_SHINY_RATE_COUNT)
            {
                gSaveBlock2Ptr->adminSettings.shinyRate = 0;
            }
        }
        else
        {
            if (gSaveBlock2Ptr->adminSettings.shinyRate == 0)
                gSaveBlock2Ptr->adminSettings.shinyRate =
                    ADMIN_SHINY_RATE_COUNT - 1;
            else
                gSaveBlock2Ptr->adminSettings.shinyRate--;
        }
        break;

    case ITEM_NUZLOCKE:
        ToggleFlag(ADMIN_FLAG_NUZLOCKE);
        break;

    case ITEM_HARDCORE:
        ToggleFlag(ADMIN_FLAG_HARDCORE);
        break;

    case ITEM_FAINT_LOCK:
        ToggleFlag(ADMIN_FLAG_FAINT_LOCK);
        break;

    case ITEM_NO_ITEMS:
        ToggleFlag(ADMIN_FLAG_NO_BATTLE_ITEMS);
        break;
    }

    DrawMenu();
}

static void Task_FadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_ProcessInput;
}

static void Task_ProcessInput(u8 taskId)
{
    if (JOY_NEW(DPAD_UP))
    {
        if (gTasks[taskId].tSelection == 0)
            gTasks[taskId].tSelection = ITEM_EXIT;
        else
            gTasks[taskId].tSelection--;

        HighlightItem(gTasks[taskId].tSelection);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (gTasks[taskId].tSelection >= ITEM_EXIT)
            gTasks[taskId].tSelection = 0;
        else
            gTasks[taskId].tSelection++;

        HighlightItem(gTasks[taskId].tSelection);
    }
    else if (JOY_NEW(DPAD_LEFT))
    {
        if (gTasks[taskId].tSelection != ITEM_EXIT)
            ChangeCurrentSetting(-1, gTasks[taskId].tSelection);
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        if (gTasks[taskId].tSelection != ITEM_EXIT)
            ChangeCurrentSetting(1, gTasks[taskId].tSelection);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        if (gTasks[taskId].tSelection == ITEM_EXIT)
        {
            BeginNormalPaletteFade(
                PALETTES_ALL, 0, 0, 16, RGB_BLACK
            );
            gTasks[taskId].func = Task_FadeOut;
        }
        else
        {
            ChangeCurrentSetting(
                1,
                gTasks[taskId].tSelection
            );
        }
    }
    else if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(
            PALETTES_ALL, 0, 0, 16, RGB_BLACK
        );
        gTasks[taskId].func = Task_FadeOut;
    }
}

static void Task_FadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(gMain.savedCallback);
    }
}
