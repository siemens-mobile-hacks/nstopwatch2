#include <swilib.h>
#include <swilib/nucleus.h>
#include <xtask_ipc.h>
#include <stdio.h>
#include <string.h>
#include "../ui/ui.h"
#include "../ipc.h"
#include "../time.h"
#include "../config_loader.h"
#include "csm.h"

typedef struct {
    CSM_RAM csm;
    STOPWATCH *stopwatch;

    int create_main_csm;
} DAEMON_CSM;

typedef struct {
    char check_name[8];
    void (*f)(short *num);
} ICONBAR_H;

int DAEMON_CSM_ID;

static const int minus11 =- 11;
static unsigned short maincsm_name_body[140];

void KillStopwatch(STOPWATCH *stopwatch) {
    NU_Control_Timer(&(stopwatch->nu_timer), NU_DISABLE_TIMER);
    NU_Delete_Timer(&(stopwatch->nu_timer));

    stopwatch->timings[0] = 0;
    stopwatch->timings[1] = GetMilliseconds();
    if (!WriteStatusToPDFile(stopwatch)) {
        MsgBoxError(1, (int)"Couldn't write status to PD file!");
    }
}

static int OnMessage(CSM_RAM *data, GBS_MSG *msg) {
    DAEMON_CSM *csm = (DAEMON_CSM*)data;
    if (msg->msg == MSG_RECONFIGURE_REQ) {
        if (strcmp(CFG_PATH, msg->data0) == 0) {
            InitConfig();
            ShowMSG(1, (int)"NStopWatch2 config updated!");
        }
    } else if (msg->msg == MSG_IPC) {
        IPC_REQ *ipc = msg->data0;
        if (strcmpi(ipc->name_to, IPC_NAME) == 0) {
            if (msg->submess == IPC_CREATE_DAEMON_CSM) {
                int ipc_csm_id = (int)ipc->data;
                if (csm->csm.id != ipc_csm_id) { // double run
                    DAEMON_CSM *daemon_csm = (DAEMON_CSM*)FindCSMbyID(ipc_csm_id);
                    if (daemon_csm) {
                        daemon_csm->create_main_csm = 1;
                    }
                    CloseCSM(ipc_csm_id);
                }
            } else if (msg->submess == IPC_CREATE_MAIN_CSM) {
                if (!MAIN_CSM_ID) {
                    if (!csm->stopwatch) {
                        csm->stopwatch = ReadPDFile();
                    }
                    SUBPROC(CSM_Create, csm->stopwatch);
                } else {
                    static IPC_REQ ipc_xtask;
                    ipc_xtask.data = (void*)MAIN_CSM_ID;
                    IPC_SendToXTask(&ipc_xtask, IPC_XTASK_SHOW_CSM);
                }
            }
        }
    }
    return 1;
}

static void OnCreate(CSM_RAM *data) {
    DAEMON_CSM *csm = (DAEMON_CSM*)data;
    csm->stopwatch = NULL;
    csm->create_main_csm = 0;
}

static void OnClose(CSM_RAM *data) {
    DAEMON_CSM *csm = (DAEMON_CSM*)data;
    if (csm->create_main_csm) {
        static IPC_REQ ipc;
        IPC_Send(&ipc, IPC_CREATE_MAIN_CSM);
        csm->create_main_csm = 0;
    } else if (MAIN_CSM_ID) {
        CloseCSM(MAIN_CSM_ID);
    }
    if (csm->stopwatch) {
        KillStopwatch(csm->stopwatch);
        DestroyStopwatch(csm->stopwatch);
    }
    SUBPROC(kill_elf);
}

static struct {
    CSM_DESC maincsm;
    WSHDR maincsm_name;
    ICONBAR_H iconbar_h;
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
        sizeof(DAEMON_CSM),
        1,
        &minus11
    },
    {
        maincsm_name_body,
        NAMECSM_MAGIC1,
        NAMECSM_MAGIC2,
        0x0,
        139,
        0
    },
    {
        {"IconBar"},
    }
};

static void UpdateCSMname(void) {
    wsprintf((WSHDR *)(&MAINCSM.maincsm_name), "NStopWatch2");
}

void AddIconBar(short *num) {
    extern int CFG_ENABLE_ICONBAR_ICON, CFG_ICON_ID;
    if (CFG_ENABLE_ICONBAR_ICON) {
        DAEMON_CSM *csm = (DAEMON_CSM*)FindCSMbyID(DAEMON_CSM_ID);
        if (csm && csm->stopwatch && csm->stopwatch->enable) {
            AddIconToIconBar(CFG_ICON_ID, num);
        }
    }
}

void Daemon_CreateCSM() {
    char dummy[sizeof(DAEMON_CSM)];
    UpdateCSMname();
    InitConfig();
    MAINCSM.iconbar_h.f = AddIconBar;
    LockSched();
    CSM_RAM *save_cmpc = CSM_root()->csm_q->current_msg_processing_csm;
    CSM_root()->csm_q->current_msg_processing_csm = CSM_root()->csm_q->csm.first;
    DAEMON_CSM_ID = CreateCSM(&MAINCSM.maincsm, dummy, 0);
    CSM_root()->csm_q->current_msg_processing_csm = save_cmpc;
    UnlockSched();

    static IPC_REQ ipc;
    ipc.data = (void*)DAEMON_CSM_ID;
    IPC_Send(&ipc, IPC_CREATE_DAEMON_CSM);
}
