#pragma once

#define IPC_NAME "NStopWatch2"

enum {
    IPC_REDRAW_TIME,
};

extern int IPC_IS_ALLOW_REDRAW_TIME;

void IPC_RedrawTime();
