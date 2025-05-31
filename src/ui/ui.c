#include <swilib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../ipc.h"
#include "../time.h"
#include "../stopwatch.h"
#include "ui.h"

#include "icons.h"
#include "menu_options.h"

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
    data->timings_size++;
    data->timings = realloc(data->timings, sizeof(t_timing) * data->timings_size); // NOLINT
    data->timings[data->timings_size - 1] = timing;
    data->first_timing_id = data->timings_size - 1;
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

int DrawTime() {
    // GUI *gui = GetTopGUI();
    // UI_DATA *data = TViewGetUserPointer(gui);

    RECT *main_area_rect = GetMainAreaRECT();
    const int font = FONT_MEDIUM_BOLD;
    int x = 0, y = main_area_rect->y + 2;
    int x2 = main_area_rect->x2, y2 = y + GetFontYSIZE(font);

    WSHDR ws;
    TTime time;
    uint16_t wsbody[32];
    CreateLocalWS(&ws, wsbody, 31);

    GetTimeFromSeconds(&time, GetMilliseconds() / 1000);
    PutTimingIntoWS(&ws, time.hour, time.min, time.sec, GetMilliseconds() % 1000);

    DrawCanvas(BuildUICanvas, x, y, x2, y2, 1);
    DrawString(&ws, x, y, x2, y2, font, TEXT_ALIGNMIDDLE,
           GetPaletteAdrByColorIndex(PC_FOREGROUND), GetPaletteAdrByColorIndex(0x17));
    return y2;
}

void DrawTimings(int y) {
    void *gui = GetTopGUI();
    UI_DATA *data = TViewGetUserPointer(gui);
    RECT *main_area_rect = GetMainAreaRECT();

    WSHDR ws;
    uint16_t wsbody[64];
    CreateLocalWS(&ws, wsbody, 63);

    int x = main_area_rect->x, x2 = main_area_rect->x2;
    int y2 = 0;

    TTime time;
    WSHDR ws_timing;
    uint16_t wsbody_timing[16];
    CreateLocalWS(&ws_timing, wsbody_timing, 15);
    for (int i = (int)data->first_timing_id; i >= 0; i--) {
        if (data->first_timing_id - i > 9 - 1) {
            break;
        }
        t_timing timing = data->timings[i];
        if (data->type == STOPWATCH_TYPE_LAP) {
            if (i > 0) {
                timing -= data->timings[i - 1];
            }
        }
        GetTimeFromMilliseconds(&time, timing);
        PutTimingIntoWS(&ws_timing, time.hour, time.min, time.sec, time.millisec);
        wsprintf(&ws, "(%02d) %w", i + 1, &ws_timing)

        y2 = y + GetFontYSIZE(FONT_SMALL);
        DrawString(&ws, x, y, x2, y2, FONT_SMALL, TEXT_ALIGNMIDDLE,
                   GetPaletteAdrByColorIndex(PC_FOREGROUND), GetPaletteAdrByColorIndex(0x17));
        y = y2;
    }
}

static void OnRedraw(GUI *) {
    GUI *gui = GetTopGUI();
    UI_DATA *data = TViewGetUserPointer(gui);

    WSHDR ws;
    uint16_t wsbody[128];
    CreateLocalWS(&ws, wsbody, 127);

    int y = DrawTime() + 2;
    DrawLine(gui->canvas->x, y, gui->canvas->x2, y, 0, GetPaletteAdrByColorIndex(PC_FOREGROUND));
    if (data->timings) {
        y += 3;
        DrawTimings(y);
    } else {
        y += 20;
        PutTimingNameIntoWS(&ws, data->type);
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
        IPC_RedrawTime();
        IPC_IS_ALLOW_REDRAW_TIME = 0;
    }
}

void StartStopwatch(GUI *gui) {
    UI_DATA *data = TViewGetUserPointer(gui);
    data->start_time = GetMillisecondsFromDateTimeSince1997();
    data->was_ever_started = 1;
    NU_Control_Timer(&(data->nu_timer), NU_ENABLE_TIMER);
}

void StopStopwatch(GUI *gui) {
    UI_DATA *data = TViewGetUserPointer(gui);

    data->start_time = 0;
    NU_Control_Timer(&(data->nu_timer), NU_DISABLE_TIMER);
}

void ResetStopwatch(GUI *gui) {
    UI_DATA *data = TViewGetUserPointer(gui);
    StopStopwatch(gui);
    if (data->timings) {
        mfree(data->timings);
    }
    data->timings = NULL;
    data->timings_size = 0;

    data->was_ever_started = 0;
    data->first_timing_id = 0;
    InitGlobalDateTime(0, 0);
    NU_Reset_Timer(&(data->nu_timer), ExpirationRoutine, 5, 5, NU_DISABLE_TIMER);
}

void SetLapTiming(GUI *gui) {
    UI_DATA *data = TViewGetUserPointer(gui);
    data->type = STOPWATCH_TYPE_LAP;
}

void SetSplitTiming(GUI *gui) {
    UI_DATA *data = TViewGetUserPointer(gui);
    data->type = STOPWATCH_TYPE_SPLIT;
}

static int OnKey(GUI *gui, GUI_MSG *msg) {
    UI_DATA *data = TViewGetUserPointer(gui);

    if (msg->keys == 0x01) {
        return 1;
    } else if (msg->keys == 0x18) {
        MenuOptions_Create(gui);
    } else if (msg->keys == 0xFF) {
        if (data->start_time) {
            double milliseconds = GetMilliseconds();
            AddTiming(data, (t_timing)milliseconds);
            DirectRedrawGUI();
        }
    } else if (msg->keys == 0x3D) {
        if (!data->start_time) {
            StartStopwatch(gui);
        } else {
            StopStopwatch(gui);
        }
        DirectRedrawGUI();
    } else if (msg->gbsmsg->msg == KEY_DOWN || msg->gbsmsg->msg == LONG_PRESS) {
        if (msg->gbsmsg->submess == UP_BUTTON) {
            if (data->first_timing_id < data->timings_size - 1) {
                data->first_timing_id++;
                DirectRedrawGUI();
            }
        } else if (msg->gbsmsg->submess == DOWN_BUTTON) {
            if (data->first_timing_id >= 9) {
                data->first_timing_id--;
                DirectRedrawGUI();
            }
        }
    }
    return -1;
}

static void GHook(GUI *gui, int cmd) {
    UI_DATA *data = TViewGetUserPointer(gui);

    if (cmd == TI_CMD_REDRAW) {
        int lsk_id = 0;
        if (data->start_time) {
            if (data->type == STOPWATCH_TYPE_LAP) {
                lsk_id = 1;
            } else {
                lsk_id = 2;
            }
        }
        SOFTKEY_D[3].lgp_id = (data->start_time) ? LGP_STOP_PIC : LGP_PLAY_PIC;

        SetMenuSoftKey(gui, &SOFTKEY_D[lsk_id], SET_LEFT_SOFTKEY);
        SetMenuSoftKey(gui, &SOFTKEY_D[3], SET_MIDDLE_SOFTKEY);
        SetMenuSoftKey(gui, &SOFTKEY_D[4], SET_RIGHT_SOFTKEY);
    }
    else if (cmd == TI_CMD_CREATE) {
        static GUI_METHODS gui_methods;
        void **m = GetDataOfItemByID(gui, 4);
        memcpy(&gui_methods, m[1], sizeof(GUI_METHODS));
        gui_methods.onRedraw = OnRedraw;
        m[1] = &gui_methods;

        NU_Create_Timer(&(data->nu_timer), "NStopWatch2", ExpirationRoutine, 0,
               5, 5, NU_DISABLE_TIMER);
    } else if (cmd == TI_CMD_FOCUS) {
        IPC_IS_ALLOW_REDRAW_TIME = 1;
    } else if (cmd == TI_CMD_UNFOCUS) {
        IPC_IS_ALLOW_REDRAW_TIME = 0;
    } else if (cmd == TI_CMD_DESTROY) {
        NU_Control_Timer(&(data->nu_timer), NU_DISABLE_TIMER);
        NU_Delete_Timer(&(data->nu_timer));

        STOPWATCH stopwatch;
        stopwatch.type = data->type;
        stopwatch.timings = malloc(sizeof(t_timing) * (data->timings_size + 3));
        stopwatch.timings[0] = 0;
        stopwatch.timings[1] = GetMilliseconds();
        memcpy(&(stopwatch.timings[2]), data->timings, sizeof(t_timing) * data->timings_size);
        stopwatch.timings[data->timings_size + 2] = -1;
        if (data->timings) {
            mfree(data->timings);
        }
        if (!WriteStatusToPDFile(&stopwatch)) {
            MsgBoxError(1, (int)"Couldn't write status to PD file!");
        }
    }
}

static TVIEW_DESC TVIEW_D = {
    8,
    OnKey,
    GHook,
    NULL,
    SOFTKEYS,
    &SOFTKEYS_TAB,
    {0, 0, 0, 0},
    FONT_MEDIUM,
    0x64,
    0x65,
    0,
    0,
};

int CreateUI(const STOPWATCH *stopwatch) {
    memcpy(&(HEADER_D.rc), GetHeaderRECT(), sizeof(RECT));
    memcpy(&(TVIEW_D.rc), GetMainAreaRECT(), sizeof(RECT));

    UI_DATA *data = malloc(sizeof(UI_DATA));
    zeromem(data, sizeof(UI_DATA));
    int global_time_offset = 0;
    int global_time_milliseconds = 0;
    if (stopwatch) {
        if (stopwatch->timings) {
            data->was_ever_started = 1;
            global_time_offset = ((int)stopwatch->timings[0]) / 1000;
            global_time_milliseconds = ((int)stopwatch->timings[0]) % 1000;

            int i = 1;
            while (stopwatch->timings[i] != -1) {
                AddTiming(data, stopwatch->timings[i++]);
            }
        }
        data->type = stopwatch->type;
    }
    InitGlobalDateTime(global_time_offset, global_time_milliseconds);

    void *mfree = mfree_adr();
    void *malloc = malloc_adr();

    WSHDR *ws = AllocWS(1);
    void *gui = TViewGetGUI(malloc, mfree);
    TViewSetDefinition(gui, &TVIEW_D);
    SetHeaderToMenu(gui, &HEADER_D, malloc);
    TViewSetText(gui, ws, malloc, mfree);
    TViewSetUserPointer(gui, data);

    data->gui_id = CreateGUI(gui);
    return data->gui_id;
}
