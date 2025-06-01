#pragma once

enum {
    IPC_CREATE_DAEMON_CSM,
    IPC_CREATE_MAIN_CSM,
    IPC_CREATE_UI,
    IPC_REDRAW_TIME,
};

extern const char *IPC_NAME;
extern int IPC_IS_ALLOW_REDRAW_TIME;

void IPC_Send(IPC_REQ *ipc, int submess);
void IPC_SendToXTask(IPC_REQ *ipc, int submess);
