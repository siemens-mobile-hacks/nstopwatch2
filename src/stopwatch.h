#pragma once

enum StopwatchType {
    STOPWATCH_TYPE_LAP,
    STOPWATCH_TYPE_SPLIT,
};

typedef uint32_t t_timing;
typedef enum StopwatchType StopwatchType;

typedef struct {
    t_timing *timings;
    StopwatchType type;
} STOPWATCH;

void DestroyStopwatch(STOPWATCH *stopwatch);

STOPWATCH *ReadPDFile();
int WriteStatusToPDFile(const STOPWATCH *stopwatch);
