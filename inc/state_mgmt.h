#ifndef __state_mgmt_H__
#define __state_mgmt_H__

#include "msg_qid_table.h"

#ifdef CONSOLE_DEBUG
#include <stdio.h>
#   define STM_DEBUG_PRINT(fmt, str...) \
        printf(\
        "%16.16s(%4d) - %16.16s: " fmt, \
        __FILE__, __LINE__, __func__, ##str)
#   define STM_ERR_PRINT(fmt, str...) \
        printf(\
        "%16.16s(%4d) - %16.16s: ERROR " fmt, \
        __FILE__, __LINE__, __func__, ##str)
#else
#   include "define.h"
#   define STM_DEBUG_PRINT PRINT_DBG
#   define STM_ERR_PRINT PRINT_ERR
#endif

#define STM_RECV_TYPE 1
typedef enum _eStmEvent
{
    eSTM_E_INIT = 0,
    eSTM_E_INIT_DONE,
    eSTM_E_SYNC,
    eSTM_E_ClOUD_CON,
    eSTM_E_FLEXWEB_CON,
    eSTM_E_ClOUD_DISCON,
    eSTM_E_FLEXWEB_DISCON,
    eSTM_E_DEINIT,
    eSTM_E_END
}eStmEvent;

typedef enum _eStmState
{
    eSTM_S_INIT = 0,
    eSTM_S_RECOV,
    eSTM_S_ClOUD,
    eSTM_S_LOCAL,
    eSTM_S_DEINIT,
    eSTM_S_ERROR,
}eStmState;

typedef struct _tStmEventMsg
{
    long type_long;
    eStmEvent event_int;
}tStmEventMsg;

/**
  * Hint :  input data  is no be free in this func
  *
  * get current status
  *
  * @return int
  *     ERROR_CODE_SUCCESS
  */
eStmState get_current_state();

/**
  * Hint :  input data  is no be free in this func
  *
  * initial STM module
  *
  * @return int
  *     ERROR_CODE_SUCCESS
  */
int initial_state_mgmt();

/**
  * Hint :  input data  is no be free in this func
  *
  * de-initial STM module
  *
  * @return int
  *     ERROR_CODE_SUCCESS
  */
int deinit_state_mgmt();

/**
  * Hint :  input data  is no be free in this func
  *
  * send evnet to notify STM chnage state
  *
  * @param event type : eStmEvent
  *     input event, see in state_mgmt.h eStmEvent
  * @return int
  *     ERROR_CODE_SUCCESS
  */
int stm_send_event(eStmEvent event);

#endif