#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#include "errorno.h"
#include "state_mgmt.h"
#include "fsm.h"

extern eStmInternalState _g_state_int;
static pthread_t state_pthread;

eStmState get_current_state()
{
    switch (_g_state_int)
    {
        case eSTM_S_ITN_INIT_NON:
        case eSTM_S_ITN_INIT_ALL:
        case eSTM_S_ITN_INIT_FLX:
        case eSTM_S_ITN_INIT_CLD:
STM_DEBUG_PRINT("eSTM_S_INIT");
            return eSTM_S_INIT;
        case eSTM_S_ITN_RECOV_NON:
        case eSTM_S_ITN_RECOV_ALL:
        case eSTM_S_ITN_RECOV_CLD:
        case eSTM_S_ITN_RECOV_FLX:
STM_DEBUG_PRINT("eSTM_S_RECOV");
            return eSTM_S_RECOV;
        case eSTM_S_ITN_ClOUD:
STM_DEBUG_PRINT("eSTM_S_ClOUD");
            return eSTM_S_ClOUD;
        case eSTM_S_ITN_LOCAL_NON:
        case eSTM_S_ITN_LOCAL_CLD:
        case eSTM_S_ITN_LOCAL_FLX:
STM_DEBUG_PRINT("eSTM_S_LOCAL");
            return eSTM_S_LOCAL;
        case eSTM_S_ITN_DEINIT:
STM_DEBUG_PRINT("eSTM_S_DEINIT");
            return eSTM_S_DEINIT;
        case eSTM_S_ITN_ERROR:
        default:
            return eSTM_S_ERROR;
    }
}

int stm_send_event(eStmEvent event)
{
    tStmEventMsg stmeve_struct;
    stmeve_struct.event_int = event;
    stmeve_struct.type_long = STM_RECV_TYPE;
    int qid_int = msgget(STATE_MGMT_QUEUE_ID, IPC_CREAT|0666);
    msgsnd(qid_int,
        &stmeve_struct,
        sizeof(tStmEventMsg)-sizeof(long),
        0
    );
    return ERROR_CODE_SUCCESS;
}

int initial_state_mgmt()
{
    _g_state_int = eSTM_S_ITN_INIT_NON;
    pthread_create(&state_pthread,
        NULL, state_machine, NULL);
    return ERROR_CODE_SUCCESS;
}

int deinit_state_mgmt()
{

    int qid_int = msgget(STATE_MGMT_QUEUE_ID, IPC_CREAT|0666);

    tStmEventMsg stmeve_struct;
    memset(&stmeve_struct, 0, sizeof(tStmEventMsg));
    stmeve_struct.event_int = eSTM_E_DEINIT;
    stmeve_struct.type_long = STM_RECV_TYPE;
    msgsnd(qid_int,
        &stmeve_struct,
        sizeof(tStmEventMsg)-sizeof(long),
        0
    );
    pthread_join(state_pthread, NULL);
    msgctl(qid_int, IPC_RMID, NULL);

    STM_DEBUG_PRINT("terminal stm");
    return ERROR_CODE_SUCCESS;
}