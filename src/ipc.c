#include <swilib.h>
#include <xtask_ipc.h>
#include "ipc.h"

const char *IPC_NAME = "NStopWatch2";
const char *IPC_NAME_XTASK = IPC_XTASK_NAME;

int IPC_IS_ALLOW_REDRAW_TIME = 1;

void IPC_Send(IPC_REQ *ipc, int submess) {
    ipc->name_to = IPC_NAME;
    ipc->name_from = IPC_NAME;
    GBS_SendMessage(MMI_CEPID, MSG_IPC, submess, ipc);
}

void IPC_SendToXTask(IPC_REQ *ipc, int submess) {
    ipc->name_to = IPC_NAME_XTASK;
    ipc->name_from = IPC_NAME_XTASK;
    GBS_SendMessage(MMI_CEPID, MSG_IPC, submess, ipc);
}
