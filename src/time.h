#pragma once

extern double MILLISECONDS;
extern int TIMESTAMP, INITIAL_TIMESTAMP;

void InitGlobalDateTime(int offset, int milliseconds);

uint32_t GetMilliseconds();
double GetMillisecondsFromDateTimeSince1997();
