#ifndef __fsm_H__
#define __fsm_H__


typedef enum _eStmInternalState
{
    eSTM_S_ITN_INIT_NON = 0,
    eSTM_S_ITN_INIT_ALL,
    eSTM_S_ITN_INIT_CLD,
    eSTM_S_ITN_INIT_FLX,
    eSTM_S_ITN_RECOV_NON,
    eSTM_S_ITN_RECOV_ALL, // 5
    eSTM_S_ITN_RECOV_CLD,
    eSTM_S_ITN_RECOV_FLX,
    eSTM_S_ITN_ClOUD,
    eSTM_S_ITN_LOCAL_NON,
    eSTM_S_ITN_LOCAL_CLD, //10
    eSTM_S_ITN_LOCAL_FLX,
    eSTM_S_ITN_DEINIT,
    eSTM_S_ITN_END,
    eSTM_S_ITN_ERROR,
}eStmInternalState;

void *state_machine(void *argc);

#endif