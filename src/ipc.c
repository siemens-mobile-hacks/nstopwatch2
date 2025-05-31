#include <swilib.h>
#include <stdlib.h>
#include "ipc.h"

int IPC_IS_ALLOW_REDRAW_TIME = 1;

static void Send(int submess) {
    IPC_REQ *ipc = malloc(sizeof(IPC_REQ));
    ipc->name_to = IPC_NAME;
    ipc->name_from = IPC_NAME;
    ipc->data = NULL;
    GBS_SendMessage(MMI_CEPID, MSG_IPC, submess, ipc);
}

void IPC_RedrawTime() {
    Send(IPC_REDRAW_TIME);
}
