#pragma once

#include <swilib/nucleus.h>

enum StopwatchType {
    STOPWATCH_TYPE_LAP,
    STOPWATCH_TYPE_SPLIT,
};

typedef uint32_t t_timing;
typedef enum StopwatchType StopwatchType;

typedef struct {
    NU_TIMER nu_timer;
    int enable;

    t_timing *timings;
    uint32_t timings_size;
    StopwatchType type;
} STOPWATCH;

void DestroyStopwatch(STOPWATCH *stopwatch);

STOPWATCH *ReadPDFile();
int WriteStatusToPDFile(const STOPWATCH *stopwatch);
