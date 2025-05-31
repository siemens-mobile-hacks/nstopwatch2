#include <swilib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stopwatch.h"

#define STOPWATCH_PD_ID 0x200C

void DestroyStopwatch(STOPWATCH *stopwatch) {
    if (stopwatch) {
        if (stopwatch->timings) {
            mfree(stopwatch->timings);
        }
        mfree(stopwatch);
    }
}

static void AddTiming(t_timing **timings, int *size, const char *s, const char *e) {
    char ms[32];
    size_t len = e - s;
    strncpy(ms, s, len);
    ms[len] = '\0';
    (*size)++;
    *timings = realloc(*timings, sizeof(t_timing) * (*size));
    (*timings)[(*size) - 1] = strtoll(ms, NULL, 10);
}

t_timing *GetTimings(const char *str, int *size) {
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
    (*size)++;
    timings = realloc(timings, sizeof(t_timing) * *size); //NOLINT
    timings[(*size) - 1] = -1;
    return timings;
}

STOPWATCH *ReadPDFile() {
    char key[32];
    char value[256];
    uint32_t len;

    len = 256;
    sprintf(key, "%s", "status");
    if (ReadValueFromPDFile(STOPWATCH_PD_ID, key, value, &len) != 0) {
        return NULL;
    }
    value[len] = '\0';

    STOPWATCH *stopwatch = malloc(sizeof(STOPWATCH));
    zeromem(stopwatch, sizeof(STOPWATCH));
    if (value[2] == 'L') {
        stopwatch->type = STOPWATCH_TYPE_LAP;
    } else {
        stopwatch->type = STOPWATCH_TYPE_SPLIT;
    }

    if (value[0] == 'I') { // Reset``
    } else {
        int size = 0;
        t_timing *timings = GetTimings(strchr(&value[4], ',') + 1, &size);
        if (size < 2) {
            mfree(timings);
        } else {
            stopwatch->timings = timings;
        }
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
    int i = 0;
    while (stopwatch->timings[i] != -1) {
        char timing[32];
        sprintf(timing, ",%d", stopwatch->timings[i++]);
        strcat(value, timing);
    }
    return (WriteValueToPDFile(STOPWATCH_PD_ID, "status", value, strlen(value)) == 0);
}
