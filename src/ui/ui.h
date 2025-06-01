#pragma once

#include "../stopwatch.h"

typedef struct {
    int gui_id;
    STOPWATCH *stopwatch;
    uint32_t first_timing_id;
} UI_DATA;

int CreateUI(STOPWATCH *stopwatch);
int DrawTime();

void StartStopwatch(GUI *gui);
void StopStopwatch(GUI *gui);
void ResetStopwatch(GUI *gui);
void SetLapTiming(GUI *gui);
void SetSplitTiming(GUI *gui);
