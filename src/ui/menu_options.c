#include <swilib.h>
#include <string.h>
#include "icons.h"
#include "ui.h"

#define ITEMS_N 6

static HEADER_DESC HEADER_D = {{0, 0, 0, 0},NULL, (int)"Options", LGP_NULL};

static const int SOFTKEYS[] = {SET_LEFT_SOFTKEY, SET_RIGHT_SOFTKEY, SET_MIDDLE_SOFTKEY};

static const SOFTKEY_DESC SOFTKEYS_D[]= {
    {0x0018,0x0000,(int)"Select"},
    {0x0001,0x0000,(int)"Back"},
    {0x003D,0x0000,(int)LGP_DOIT_PIC}
};

static const SOFTKEYSTAB SOFTKEYS_TAB = {
    SOFTKEYS_D,0
};

enum MenuItems {
    MENU_ITEM_START,
    MENU_ITEM_RESUME,
    MENU_ITEM_RESET,
    MENU_ITEM_LAP_TIMING,
    MENU_ITEM_SPLIT_TIMING,
    MENU_ITEM_EXIT,
};

static void Start_Proc(GUI *gui) {
    void *data = MenuGetUserPointer(gui);
    StartStopwatch(data);
    GeneralFuncF1(1);
}

static void Reset_Proc(GUI *gui) {
    void *data = MenuGetUserPointer(gui);
    ResetStopwatch(data);
    GeneralFuncF1(1);
}

static void LapTiming_Proc(GUI *gui) {
    void *data = MenuGetUserPointer(gui);
    SetLapTiming(data);
    GeneralFuncF1(1);
}

static void SplitTiming_Proc(GUI *gui) {
    void *data = MenuGetUserPointer(gui);
    SetSplitTiming(data);
    GeneralFuncF1(1);
}

static void Exit_Proc(GUI *gui) {
    void *data = MenuGetUserPointer(gui);
    UI_DATA *ui_data = TViewGetUserPointer(data);
    GeneralFuncF1(1);
    GeneralFunc_flag1(ui_data->gui_id, 1);
}

static int ICON[] = {ICON_EMPTY};

static MENUITEM_DESC ITEMS[ITEMS_N] = {
    {ICON, (int)"Start", LGP_NULL, 0, NULL, MENU_FLAG3, MENU_FLAG2},
    {ICON, (int)"Resume", LGP_NULL, 0, NULL, MENU_FLAG3, MENU_FLAG2},
    {ICON, (int)"Reset", LGP_NULL, 0, NULL, MENU_FLAG3, MENU_FLAG2},
    {ICON, (int)"Lap timing", LGP_NULL, 0, NULL, MENU_FLAG3, MENU_FLAG2},
    {ICON, (int)"Split timing", LGP_NULL, 0, NULL, MENU_FLAG3, MENU_FLAG2},
    {ICON, (int)"Exit", LGP_NULL, 0, NULL, MENU_FLAG3, MENU_FLAG2},
};

static const MENUPROCS_DESC PROCS[ITEMS_N] =  {
    Start_Proc,
    Start_Proc,
    Reset_Proc,
    LapTiming_Proc,
    SplitTiming_Proc,
    Exit_Proc,
};

static const MENU_DESC MENU_D = {
    8,
    NULL,
    NULL,
    NULL,
    SOFTKEYS,
    &SOFTKEYS_TAB,
    MENU_FLAGS_ENABLE_TEXT_SCROLLING | MENU_FLAGS_ENABLE_ICONS,
    NULL,
    ITEMS,
    PROCS,
    ITEMS_N
};

int MenuOptions_Create(GUI *gui) {
    UI_DATA *ui_data = TViewGetUserPointer(gui);

    RECT *rc = GetOptionsHeaderRect();
    memcpy(&(HEADER_D.rc), rc, sizeof(RECT));

    int items_count = 0;
    int to_remove[ITEMS_N + 1];

    if (ui_data->stopwatch->timings[1]) {
        to_remove[++items_count] = MENU_ITEM_START;
    } else {
        to_remove[++items_count] = MENU_ITEM_RESUME;
    }
    if (ui_data->stopwatch->type == STOPWATCH_TYPE_LAP) {
        to_remove[++items_count] = MENU_ITEM_LAP_TIMING;
    } else {
        to_remove[++items_count] = MENU_ITEM_SPLIT_TIMING;
    }
    to_remove[0] = items_count;

    return CreateMenu(1, 0, &MENU_D, &HEADER_D, 0, items_count, gui, to_remove);
}
