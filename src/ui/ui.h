#pragma once

#include <swilib/nucleus.h>
#include "../stopwatch.h"

typedef struct {
    int gui_id;
    NU_TIMER nu_timer;

    t_timing *timings;
    uint32_t timings_size;

    double start_time;
    int was_ever_started;
    uint32_t first_timing_id;

    StopwatchType type;
} UI_DATA;

int CreateUI(const STOPWATCH *stopwatch);
int DrawTime();

void StartStopwatch(GUI *gui);
void StopStopwatch(GUI *gui);
void ResetStopwatch(GUI *gui);
void SetLapTiming(GUI *gui);
void SetSplitTiming(GUI *gui);
