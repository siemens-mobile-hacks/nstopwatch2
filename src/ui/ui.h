#pragma once

#include "../stopwatch.h"

typedef struct {
    int gui_id;
    STOPWATCH *stopwatch;
    uint32_t timing_id;
} UI_DATA;

int CreateUI(STOPWATCH *stopwatch);
void UpdateTime(GUI *gui);
void RedrawTime(GUI *gui);

void StartStopwatch(GUI *gui);
void StopStopwatch(GUI *gui);
void ResetStopwatch(GUI *gui);
void SetLapTiming(GUI *gui);
void SetSplitTiming(GUI *gui);
