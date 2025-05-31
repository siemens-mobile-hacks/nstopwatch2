#include <swilib.h>
#include "ui/ui.h"
#include "ipc.h"
#include "stopwatch.h"

typedef struct {
    CSM_RAM csm;
    int gui_id;
} MAIN_CSM;

const int minus11 =- 11;
unsigned short maincsm_name_body[20];

static int CSM_OnMessage(CSM_RAM *data, GBS_MSG *msg) {
    MAIN_CSM *csm = (MAIN_CSM*)data;
    if (msg->msg == MSG_GUI_DESTROYED) {
        if ((int)msg->data0 == csm->gui_id) {
            csm->gui_id = 0;
            csm->csm.state =- 3;
        }
    } else if (msg->msg == MSG_IPC) {
        IPC_REQ *ipc = msg->data0;
        if (strcmpi(ipc->name_to, IPC_NAME) == 0) {
            if (msg->submess == IPC_REDRAW_TIME) {
                if (IsGuiOnTop(csm->gui_id)) {
                    DrawTime();
                    IPC_IS_ALLOW_REDRAW_TIME = 1;
                }
            }
            mfree(ipc);
        }
    }
    return 1;
}

static void CSM_OnCreate(CSM_RAM *data) {
    MAIN_CSM *csm = (MAIN_CSM*)data;
    STOPWATCH *stopwatch = ReadPDFile();
    if (stopwatch) {
        csm->gui_id = CreateUI(stopwatch);
    } else {
        csm->gui_id = CreateUI(NULL);
    }
    DestroyStopwatch(stopwatch);
}

static void CSM_OnClose(CSM_RAM *data) {
    SUBPROC(kill_elf);
}

const struct {
    CSM_DESC maincsm;
    WSHDR maincsm_name;
} MAINCSM = {
    {
        CSM_OnMessage,
        CSM_OnCreate,
#ifdef NEWSGOLD
        0,
        0,
        0,
        0,
#endif
        CSM_OnClose,
        sizeof(MAIN_CSM),
        1,
        &minus11
    },
    {
        maincsm_name_body,
        NAMECSM_MAGIC1,
        NAMECSM_MAGIC2,
        0x0,
        139
    }
};

void UpdateCSMname(void) {
    wsprintf((WSHDR *)(&MAINCSM.maincsm_name), "%s", "Stopwatch");
}

int main() {
    MAIN_CSM csm;
    UpdateCSMname();
    LockSched();
    CreateCSM(&MAINCSM.maincsm, &csm, 0);
    UnlockSched();
    return 0;
}
