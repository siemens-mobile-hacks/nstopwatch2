#include <swilib.h>
#include "time.h"

double MILLISECONDS, INITIAL_MILLISECONDS;
int TIMESTAMP, INITIAL_TIMESTAMP;

void InitGlobalDateTime(int offset, int milliseconds) {
    TTime time;
    TDate date;
    GetDateTime(&date, &time);
    TIMESTAMP = INITIAL_TIMESTAMP = GetSecondsFromDateTimeSince1997(&INITIAL_TIMESTAMP, &date, &time);
    TIMESTAMP += offset;
    MILLISECONDS = INITIAL_MILLISECONDS = (double)milliseconds;
}

uint32_t GetMilliseconds() {
    return (TIMESTAMP - INITIAL_TIMESTAMP) * 1000 + (uint32_t)MILLISECONDS;
}

double GetMillisecondsFromDateTimeSince1997() {
    return (double)TIMESTAMP * 1000.0 + MILLISECONDS;
}
