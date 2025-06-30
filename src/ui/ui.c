#include <swilib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../ipc.h"
#include "../time.h"
#include "ui.h"
#include "icons.h"
#include "menu_options.h"

#define VISIBLE_TIMINGS 8
#define SCROLLBAR_WIDGET_ID 3

GUI_METHODS METHODS;

static int ICON[] = {ICON_STOPWATCH};

static HEADER_DESC HEADER_D = {{0, 0, 0, 0}, ICON, (int)"Stopwatch", LGP_NULL};

const int SOFTKEYS[] = {SET_LEFT_SOFTKEY, SET_MIDDLE_SOFTKEY, SET_RIGHT_SOFTKEY };

static SOFTKEY_DESC SOFTKEY_D[] = {
    {0x0018, 0x0000, (int)"Options"},
    {0x00FF, 0x0000, (int)"Lap"},
    {0x00FF, 0x0000, (int)"Split"},
    {0x003D, 0x0000, (int)LGP_DOIT_PIC},
    {0x0001, 0x0000, (int)"Exit"},
};

static SOFTKEYSTAB SOFTKEYS_TAB = {
    SOFTKEY_D, 0
};

static void AddTiming(UI_DATA *data, t_timing timing) {
    data->stopwatch->timings_size++;
    data->stopwatch->timings = realloc(data->stopwatch->timings, sizeof(t_timing) * data->stopwatch->timings_size); // NOLINT
    data->stopwatch->timings[data->stopwatch->timings_size - 1] = timing;
    data->timing_id = data->stopwatch->timings_size - 1;
}

int BuildUICanvas(const RECT *rc, int flag) {
    if (flag) {
        IMGHDR *img = GetIMGHDRFromThemeCache(BODY_STANDART);
        const int height = rc->y2 - rc->y;
        DrawCroppedIMGHDR(rc->x, rc->y, 0, 0, rc->x2 - rc->x, height, 0, img);
        return 1;
    } else {
        return 0;
    }
}

void PutTimingIntoWS(WSHDR *ws, uint8_t hours, uint8_t minutes, uint8_t seconds, double milliseconds) {
    char str[32];
    sprintf(str, "%02d:%02d:%02d.%03.0lf", hours, minutes, seconds, milliseconds);
    str_2ws(ws, str, 31);
}

void PutTimingNameIntoWS(WSHDR *ws, StopwatchType type) {
    char str[32];
    if (type == STOPWATCH_TYPE_LAP) {
        strcpy(str, "Lap timing");
    } else {
        strcpy(str, "Split timing");
    }
    str_2ws(ws, str, 31);
}

void UpdateTime(GUI *gui) {
    WSHDR ws;
    TTime time;
    uint16_t wsbody[32];
    CreateLocalWS(&ws, wsbody, 31);

    GetTimeFromSeconds(&time, GetMilliseconds() / 1000);
    PutTimingIntoWS(&ws, time.hour, time.min, time.sec, GetMilliseconds() % 1000);
    EDIT_SetTextToFocused(gui, &ws);
}

void RedrawTime(GUI *gui) {
    WIDGET *widget = GetDataOfItemByID(gui, 4);
    WIDGET *time = GetDataOfItemByID(widget, 4);
    DrawCanvas(BuildUICanvas, time->canvas->x, time->canvas->y, time->canvas->x2, time->canvas->y + GetFontYSIZE(FONT_MEDIUM) + 12, 1);
    time->methods->onRedraw(time);
}

void DrawTimings(GUI *gui) {
    UI_DATA *data = EDIT_GetUserPointer(gui);

    WSHDR ws;
    uint16_t wsbody[64];
    CreateLocalWS(&ws, wsbody, 63);

    WIDGET *input = GetDataOfItemByID(gui, 4);
    int x = input->canvas->x, x2 = input->canvas->x2;
    int y = input->canvas->y + 54, y2 = 0;

    TTime time;
    WSHDR ws_timing;
    uint16_t wsbody_timing[16];
    CreateLocalWS(&ws_timing, wsbody_timing, 15);
    for (uint32_t i = data->timing_id; i >= 2; i--) {
        if (data->timing_id - i > VISIBLE_TIMINGS - 1) {
            break;
        }
        t_timing timing = data->stopwatch->timings[i];
        if (data->stopwatch->type == STOPWATCH_TYPE_LAP) {
            if (i > 2) {
                timing -= data->stopwatch->timings[i - 1];
            }
        }
        GetTimeFromMilliseconds(&time, timing);
        PutTimingIntoWS(&ws_timing, time.hour, time.min, time.sec, time.millisec);
        wsprintf(&ws, "(%02d) %w", i - 1, &ws_timing)

        y2 = y + GetFontYSIZE(FONT_SMALL);
        DrawString(&ws, x, y, x2, y2, FONT_SMALL, TEXT_ALIGNMIDDLE,
                   GetPaletteAdrByColorIndex(PC_FOREGROUND), GetPaletteAdrByColorIndex(0x17));
        y = y2;
    }
}

static void OnRedraw(GUI *gui) {
    UI_DATA *data = EDIT_GetUserPointer(gui);

    WIDGET *input = GetDataOfItemByID(gui, 4);
    input->methods->onRedraw(input);

    INPUTDIA_DESC *input_desc = gui->definition;
    input_desc->global_hook_proc(gui, TI_CMD_REDRAW);
    for (int i = 0; i < 3; i++) {
        WIDGET *widget = GetDataOfItemByID(gui, i);
        if (widget) {
            widget->methods->onRedraw(widget);
        }
    }

    int y = input->canvas->y + 46;
    DrawLine(input->canvas->x, y, input->canvas->x2, y, 0, GetPaletteAdrByColorIndex(PC_FOREGROUND));

    if (data->timing_id > VISIBLE_TIMINGS) {
        WIDGET *scrollbar = GetDataOfItemByID(gui, SCROLLBAR_WIDGET_ID);
        if (scrollbar) {
            int limit = data->stopwatch->timings_size - 1 - VISIBLE_TIMINGS;
            int value = data->stopwatch->timings_size - 1 - data->timing_id;
            if (limit > 1) {
                Scrollbar_Configure(scrollbar, limit, 0, 0);
                Scrollbar_SetValue(scrollbar, value);
                scrollbar->methods->onRedraw(scrollbar);
            }
        }
    }

    WSHDR ws;
    uint16_t wsbody[32];
    CreateLocalWS(&ws, wsbody, 31);
    if (data->stopwatch->timings_size > 2) {
        y += 8;
        DrawTimings(gui);
    } else {
        y += 20;
        PutTimingNameIntoWS(&ws, data->stopwatch->type);
        DrawString(&ws, gui->canvas->x, y, gui->canvas->x2, y + GetFontYSIZE(FONT_MEDIUM), FONT_MEDIUM,
                   TEXT_ALIGNMIDDLE, GetPaletteAdrByColorIndex(PC_FOREGROUND), GetPaletteAdrByColorIndex(0x17));
    }
}

static void ExpirationRoutine() {
    double ms = MILLISECONDS + OS_TICK_PERIOD_US / 100.0 / 2;
    if (ms >= 1000) {
        TIMESTAMP++;
        ms -= 1000;
    }
    MILLISECONDS = ms;
    if (IPC_IS_ALLOW_REDRAW_TIME) {
        static IPC_REQ ipc;
        IPC_IS_ALLOW_REDRAW_TIME = 0;
        IPC_Send(&ipc, IPC_REDRAW_TIME);
    }
}

void StartStopwatch(GUI *gui) {
    UI_DATA *data = EDIT_GetUserPointer(gui);

    data->stopwatch->enable = 1;
    data->stopwatch->timings[1] = GetMilliseconds();
    NU_Control_Timer(&(data->stopwatch->nu_timer), NU_ENABLE_TIMER);
}

void StopStopwatch(GUI *gui) {
    UI_DATA *data = EDIT_GetUserPointer(gui);

    data->stopwatch->enable = 0;
    NU_Control_Timer(&(data->stopwatch->nu_timer), NU_DISABLE_TIMER);
}

void ResetStopwatch(GUI *gui) {
    UI_DATA *data = EDIT_GetUserPointer(gui);

    StopStopwatch(gui);
    data->stopwatch->timings_size = 2;
    data->stopwatch->timings = realloc(data->stopwatch->timings, sizeof(t_timing) * data->stopwatch->timings_size); //NOLINT
    data->stopwatch->timings[0] = 0;
    data->stopwatch->timings[1] = 0;
    data->timing_id = data->stopwatch->timings_size - 1;
    InitGlobalDateTime(0, 0);
    NU_Reset_Timer(&(data->stopwatch->nu_timer), ExpirationRoutine, 5, 5, NU_DISABLE_TIMER);
    UpdateTime(gui);
}

void SetLapTiming(GUI *gui) {
    UI_DATA *data = EDIT_GetUserPointer(gui);

    data->stopwatch->type = STOPWATCH_TYPE_LAP;
}

void SetSplitTiming(GUI *gui) {
    UI_DATA *data = EDIT_GetUserPointer(gui);

    data->stopwatch->type = STOPWATCH_TYPE_SPLIT;
}

static int OnKey(GUI *gui, GUI_MSG *msg) {
    UI_DATA *data = EDIT_GetUserPointer(gui);

    if (msg->keys == 0x01) {
        return 1;
    } else if (msg->keys == 0x18) {
        MenuOptions_Create(gui);
    } else if (msg->keys == 0xFF) {
        if (data->stopwatch->enable) {
            double milliseconds = GetMilliseconds();
            AddTiming(data, (t_timing)milliseconds);
        }
    } else if (msg->keys == 0x3D) {
        if (!data->stopwatch->enable) {
            StartStopwatch(gui);
        } else {
            StopStopwatch(gui);
        }
    } else if (msg->gbsmsg->msg == KEY_DOWN || msg->gbsmsg->msg == LONG_PRESS) {
        if (msg->gbsmsg->submess == UP_BUTTON) {
            if (data->timing_id < data->stopwatch->timings_size - 1) {
                data->timing_id++;
            }
        } else if (msg->gbsmsg->submess == DOWN_BUTTON) {
            if (data->timing_id >= VISIBLE_TIMINGS + 2) {
                data->timing_id--;
            }
        }
    }
    return -1;
}

static void GHook(GUI *gui, int cmd) {
    UI_DATA *data = EDIT_GetUserPointer(gui);

    if (cmd == TI_CMD_REDRAW) {
        UpdateTime(gui);
        int lsk_id = 0;
        if (data->stopwatch->enable) {
            if (data->stopwatch->type == STOPWATCH_TYPE_LAP) {
                lsk_id = 1;
            } else {
                lsk_id = 2;
            }
        }
        SOFTKEY_D[3].lgp_id = (data->stopwatch->enable) ? LGP_STOP_PIC : LGP_PLAY_PIC;

        SetSoftKey(gui, &SOFTKEY_D[lsk_id], SET_LEFT_SOFTKEY);
        SetSoftKey(gui, &SOFTKEY_D[3], SET_MIDDLE_SOFTKEY);
        SetSoftKey(gui, &SOFTKEY_D[4], SET_RIGHT_SOFTKEY);
    }
    else if (cmd == TI_CMD_CREATE) {
        NU_Create_Timer(&(data->stopwatch->nu_timer), "NStopWatch2", ExpirationRoutine, 0,
               5, 5, NU_DISABLE_TIMER);
        UpdateTime(gui);
        data->timing_id = data->stopwatch->timings_size - 1;
    } else if (cmd == TI_CMD_FOCUS) {
        IPC_IS_ALLOW_REDRAW_TIME = 1;
    } else if (cmd == TI_CMD_UNFOCUS) {
        IPC_IS_ALLOW_REDRAW_TIME = 0;
    } else if (cmd == TI_CMD_DESTROY) {
        mfree(data);
    }
}

static INPUTDIA_DESC INPUTDIA_D = {
    8,
    OnKey,
    GHook,
    NULL,
    0,
    &SOFTKEYS_TAB,
    {0, 0, 0, 0},
    FONT_MEDIUM,
    0x64,
    0x65,
    0,
    TEXT_ALIGNMIDDLE,
    INPUTDIA_FLAGS_SWAP_SOFTKEYS,
};

static SCROLLBAR_DESC SCROLLBAR_D = {
    {0, 0, 0, 0}
};

int CreateUI(STOPWATCH *stopwatch) {
    RECT *main_area_rect = GetMainAreaRECT();

    memcpy(&(HEADER_D.rc), GetHeaderRECT(), sizeof(RECT));
    memcpy(&(INPUTDIA_D.rc), main_area_rect, sizeof(RECT));

    UI_DATA *data = malloc(sizeof(UI_DATA));
    zeromem(data, sizeof(UI_DATA));
    if (!INITIAL_TIMESTAMP) {
        int global_time_offset = 0;
        int global_time_milliseconds = 0;
        if (stopwatch->timings[1]) {
            global_time_offset = ((int)stopwatch->timings[1]) / 1000;
            global_time_milliseconds = ((int)stopwatch->timings[1]) % 1000;
        }
        InitGlobalDateTime(global_time_offset, global_time_milliseconds);
    }
    data->stopwatch = stopwatch;

    void *ma = malloc_adr();
    void *mf = mfree_adr();
    EDITQ *eq = AllocEQueue(ma, mf);

    WSHDR ws;
    uint16_t wsbody[128];
    CreateLocalWS(&ws, wsbody, 127);

    EDITCONTROL ec;

    ConstructEditControl(&ec, ECT_READ_ONLY, ECF_APPEND_EOL, &ws, 32);
    AddEditControlToEditQend(eq, &ec, ma);

    ConstructEditControl(&ec, ECT_READ_ONLY, ECF_NORMAL_STR, &ws, 0);
    AddEditControlToEditQend(eq, &ec, ma);

    GUI *gui = EDIT_GetGUI(ma, mf);
    EDIT_SetDefinition(gui, &INPUTDIA_D);
    EDIT_SetEQueue(gui, eq, 1, ma);
    SetHeader(gui, &HEADER_D, ma);
    EDIT_SetUserPointer(gui, data);

    WIDGET *scrollbar = CreateScrollbarWidget(ma);
    SCROLLBAR_D.rc.x = main_area_rect->x2 - 3;
    SCROLLBAR_D.rc.y = main_area_rect->y + 46 + 10;
    SCROLLBAR_D.rc.x2 = main_area_rect->x2 - 2;
    SCROLLBAR_D.rc.y2 = main_area_rect->y2 - 2;
    SetGUIDefinition(scrollbar, &SCROLLBAR_D);
    AttachWidget(gui, scrollbar, SCROLLBAR_WIDGET_ID, ma);

    memcpy(&METHODS, gui->methods, sizeof(GUI_METHODS));
    METHODS.onRedraw = OnRedraw;
    gui->methods = &METHODS;

    data->gui_id = CreateGUI(gui);
    return data->gui_id;
}
