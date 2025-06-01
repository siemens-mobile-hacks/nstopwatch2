#include <swilib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stopwatch.h"

#define STOPWATCH_PD_ID 0x200C

void DestroyStopwatch(STOPWATCH *stopwatch) {
    if (stopwatch->timings) {
        mfree(stopwatch->timings);
    }
    mfree(stopwatch);
}

static void AddTiming(t_timing **timings, uint32_t *size, const char *s, const char *e) {
    char ms[32];
    size_t len = e - s;
    strncpy(ms, s, len);
    ms[len] = '\0';
    (*size)++;
    *timings = realloc(*timings, sizeof(t_timing) * (*size));
    (*timings)[(*size) - 1] = strtoll(ms, NULL, 10);
}

t_timing *GetTimings(const char *str, uint32_t *size) {
    char *p = (char*)str;
    t_timing *timings = NULL;
    *size = 0;
    while (*p) {
        char *e = strchr(p, ',');
        if (!e) {
            e = strchr(p, '\0');
            AddTiming(&timings, size, p, e);
            break;
        }
        AddTiming(&timings, size, p, e);
        p = e + 1;
    }
    if (*size < 2) {
        timings = realloc(timings, sizeof(t_timing) * 2); // NOLINT
        timings[0] = 0;
        timings[1] = 0;
        *size = 2;
    }
    return timings;
}

STOPWATCH *ReadPDFile() {
    STOPWATCH *stopwatch = malloc(sizeof(STOPWATCH));
    zeromem(stopwatch, sizeof(STOPWATCH));

    char key[32];
    char value[256];
    uint32_t len;

    len = 256;
    sprintf(key, "%s", "status");
    if (ReadValueFromPDFile(STOPWATCH_PD_ID, key, value, &len) != 0) {
        stopwatch->type = STOPWATCH_TYPE_LAP;
        stopwatch->timings = GetTimings(NULL, &(stopwatch->timings_size));
        return stopwatch;
    }
    value[len] = '\0';

    if (value[2] == 'L') {
        stopwatch->type = STOPWATCH_TYPE_LAP;
    } else {
        stopwatch->type = STOPWATCH_TYPE_SPLIT;
    }

    if (value[0] == 'I') { // Reset
        stopwatch->timings = GetTimings(NULL, &(stopwatch->timings_size));
    } else {
        stopwatch->timings = GetTimings(&value[4], &(stopwatch->timings_size));
    }
    return stopwatch;
}

int WriteStatusToPDFile(const STOPWATCH *stopwatch) {
    char value[1024];
    sprintf(value, "S,L");
    if (stopwatch->timings[1] == 0) {
        value[0] = 'I'; //Reset
    } else {
        value[0] = 'S'; // Stop
    }
    if (stopwatch->type == STOPWATCH_TYPE_LAP) {
        value[2] = 'L'; // Lap
    } else {
        value[2] = 'S'; // Split
    }
    for (int i = 0; i < stopwatch->timings_size; i++) {
        char timing[32];
        sprintf(timing, ",%d", stopwatch->timings[i]);
        strcat(value, timing);
    }
    return (WriteValueToPDFile(STOPWATCH_PD_ID, "status", value, strlen(value)) == 0);
}
