#include <swilib.h>
#include "../ui/ui.h"
#include "../ipc.h"

typedef struct {
    CSM_RAM csm;
    int gui_id;
} MAIN_CSM;

int MAIN_CSM_ID;

static const int minus11 =- 11;
static unsigned short maincsm_name_body[20];

static int OnMessage(CSM_RAM *data, GBS_MSG *msg) {
    MAIN_CSM *csm = (MAIN_CSM*)data;
    if (msg->msg == MSG_GUI_DESTROYED) {
        if ((int)msg->data0 == csm->gui_id) {
            csm->gui_id = 0;
            csm->csm.state = -3;
        }
    } else if (msg->msg == MSG_IPC) {
        IPC_REQ *ipc = msg->data0;
        if (strcmpi(ipc->name_to, IPC_NAME) == 0) {
            if (msg->submess == IPC_CREATE_UI) {
                csm->gui_id = CreateUI(ipc->data);
            } else if (msg->submess == IPC_REDRAW_TIME) {
                if (IsGuiOnTop(csm->gui_id)) {
                    GUI *gui = FindGUIbyId(csm->gui_id, &data);
                    if (gui) {
                        UpdateTime(gui);
                        RedrawTime(gui);
                    }
                    IPC_IS_ALLOW_REDRAW_TIME = 1;
                }
            }
        }
    }
    return 1;
}

static void OnCreate(CSM_RAM *data) {
}

static void OnClose(CSM_RAM *data) {
    MAIN_CSM_ID = 0;
}

static const struct {
    CSM_DESC maincsm;
    WSHDR maincsm_name;
} MAINCSM = {
    {
        OnMessage,
        OnCreate,
#ifdef NEWSGOLD
        0,
        0,
        0,
        0,
#endif
        OnClose,
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

static void UpdateCSMname(void) {
    wsprintf((WSHDR *)(&MAINCSM.maincsm_name), "%s", "Stopwatch");
}

void CSM_Create(STOPWATCH *stopwatch) {
    MAIN_CSM csm;
    UpdateCSMname();
    LockSched();
    MAIN_CSM_ID = CreateCSM(&MAINCSM.maincsm, &csm, 0);
    UnlockSched();

    static IPC_REQ ipc;
    ipc.data = stopwatch;
    IPC_Send(&ipc, IPC_CREATE_UI);
}
