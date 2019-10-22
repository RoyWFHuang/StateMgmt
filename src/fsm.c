#include "state_mgmt.h"
#include "fsm.h"
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define MAX_EVENT eSTM_E_END
#define MAX_STATE eSTM_S_ITN_END
/*
static int (* state_table[MAX_EVENT][MAX_STATE])(eStmEvent event_int) = {
{init_state, 0, 0, 0, 0, 0, 0, 0, 0, deinit_state},
{init_state, 0, 0, 0, 0, 0, 0, 0, 0, deinit_state },
{0, recovery_state, recovery_done_state, 0, 0, 0, 0, 0, 0, deinit_state},
{0, 0, recovery_done_state, recovery_con_cloud_state, recovery_con_flexweb_state, cloud_state, local_state, 0, local_con_flexweb_state, deinit_state},
{0, 0, recovery_done_state, recovery_con_cloud_state, recovery_con_flexweb_state, cloud_state, local_state, local_con_cloud_state, 0, deinit_state},
{0, 0, recovery_done_state, recovery_con_cloud_state, recovery_con_flexweb_state, cloud_state, 0, local_con_cloud_state, 0, deinit_state},
{0, 0, recovery_done_state, recovery_con_cloud_state, recovery_con_flexweb_state, cloud_state, 0, 0, local_con_flexweb_state, deinit_state},
{init_state, recovery_state, recovery_done_state, recovery_con_cloud_state, recovery_con_flexweb_state, cloud_state, local_state, local_con_cloud_state, local_con_flexweb_state, deinit_state},
};
*/


static int init_con_non_state(eStmEvent event_int);
static int init_con_all_state(eStmEvent event_int);
static int init_con_cloud_state(eStmEvent event_int);
static int init_con_flexweb_state(eStmEvent event_int);
static int recovery_con_non_state(eStmEvent event_int);
static int recovery_con_all_state(eStmEvent event_int);
static int recovery_con_cloud_state(eStmEvent event_int);
static int recovery_con_flexweb_state(eStmEvent event_int);
static int cloud_state(eStmEvent event_int);
static int local_con_non_state(eStmEvent event_int);
static int local_con_cloud_state(eStmEvent event_int);
static int local_con_flexweb_state(eStmEvent event_int);
static int deinit_state(eStmEvent event_int);
//static int error_state(eStmEvent event_int);


static int (* state_table[MAX_STATE])(eStmEvent event_int) =
{                           //eSTM_S_ITN_PRE_INIT
    init_con_non_state,         //eSTM_S_ITN_INIT_NON
    init_con_all_state,         //eSTM_S_ITN_INIT_ALL
    init_con_cloud_state,         //eSTM_S_ITN_INIT_CLD
    init_con_flexweb_state,         //eSTM_S_ITN_INIT_FLX
    recovery_con_non_state,     //eSTM_S_ITN_RECOV_NON
    recovery_con_all_state,     //eSTM_S_ITN_RECOV_ALL
    recovery_con_cloud_state,   //eSTM_S_ITN_RECOV_CLD
    recovery_con_flexweb_state, //eSTM_S_ITN_RECOV_FLX
    cloud_state,                //eSTM_S_ITN_ClOUD
    local_con_non_state,        //eSTM_S_ITN_LOCAL_NON
    local_con_cloud_state,      //eSTM_S_ITN_LOCAL_CLD
    local_con_flexweb_state,    //eSTM_S_ITN_LOCAL_FLX
    deinit_state                //eSTM_S_ITN_DEINIT
};


eStmInternalState _g_state_int;
//static pthread_t event_pthread;
void *state_machine(void *argc)
{

    tStmEventMsg msg_struct;
    int qid_int = msgget(STATE_MGMT_QUEUE_ID, IPC_CREAT|0666);
    int exit_flag = 0;
    //int corrent_state_int = eSTM_S_ITN_INIT_NON;
    while(!exit_flag)
    {
#ifdef STM_DEBUG_MODE
    STM_DEBUG_PRINT("0:eSTM_S_ITN_INIT_NON,   1:eSTM_S_ITN_INIT_ALL");
    STM_DEBUG_PRINT("2:eSTM_S_ITN_INIT_CLD,   3:eSTM_S_ITN_INIT_FLX");
    STM_DEBUG_PRINT("4:eSTM_S_ITN_RECOV_NON,  5:eSTM_S_ITN_RECOV_ALL");
    STM_DEBUG_PRINT("6:eSTM_S_ITN_RECOV_CLD,  7:eSTM_S_ITN_RECOV_FLX");
    STM_DEBUG_PRINT("8:eSTM_S_ITN_ClOUD,      9:eSTM_S_ITN_LOCAL_NON");
    STM_DEBUG_PRINT("10:eSTM_S_ITN_LOCAL_CLD, 11:eSTM_S_ITN_LOCAL_FLX");
    STM_DEBUG_PRINT("12:eSTM_S_ITN_DEINIT,    13:eSTM_S_ITN_END");
    STM_DEBUG_PRINT("14:eSTM_S_ITN_ERROR");
#endif
        STM_DEBUG_PRINT("current state[%d]\n", _g_state_int);
        memset(&msg_struct, 0, sizeof(tStmEventMsg));
        int ret_int = msgrcv(qid_int, &msg_struct,
            sizeof(tStmEventMsg)-sizeof(long), STM_RECV_TYPE, 0);
        if(ret_int == -1)
        {
            continue;
        }


#ifdef STM_DEBUG_MODE
        STM_DEBUG_PRINT("0:eSTM_E_INIT,           1:eSTM_E_INIT_DONE,   \
2:eSTM_E_SYNC\n");
        STM_DEBUG_PRINT("3:eSTM_E_ClOUD_CON,      4:eSTM_E_FLEXWEB_CON, \
5:eSTM_E_ClOUD_DISCON\n");
        STM_DEBUG_PRINT("6:eSTM_E_FLEXWEB_DISCON, 7:eSTM_E_DEINIT\n");
#endif
        STM_DEBUG_PRINT("msg_struct.event_int[%d]\n", msg_struct.event_int);
        if( (_g_state_int >= eSTM_S_ITN_INIT_NON &&
                _g_state_int < eSTM_S_ITN_END ) &&
            (msg_struct.event_int >= eSTM_E_INIT &&
                msg_struct.event_int < eSTM_E_END))
        {
            state_table[_g_state_int](
                    msg_struct.event_int);
            /*
            if(0 != state_table[msg_struct.event_int][corrent_state_int])
            {
                corrent_state_int =
                state_table[msg_struct.event_int][corrent_state_int](
                    msg_struct.event_int);
            }
            else
            {
                STM_DEBUG_PRINT("state[%d], error event[%d] \n",
                    corrent_state_int,
                    msg_struct.event_int);
            }
            */
            if(eSTM_E_DEINIT == msg_struct.event_int)
                break;
        }

    }
    return NULL;
}

static int init_con_non_state(eStmEvent event_int)
{

    switch(event_int)
    {
        case eSTM_E_INIT_DONE:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_RECOV_NON);
            //send_start_recovery();
            return eSTM_S_ITN_RECOV_NON;
        case eSTM_E_DEINIT:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_DEINIT);
            return eSTM_S_ITN_DEINIT;
        case eSTM_E_ClOUD_CON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_INIT_CLD);
            return eSTM_S_ITN_INIT_CLD;
        case eSTM_E_FLEXWEB_CON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_INIT_FLX);
            return eSTM_S_ITN_INIT_FLX;
        case eSTM_E_INIT:
        case eSTM_E_SYNC:
        case eSTM_E_ClOUD_DISCON:
        case eSTM_E_FLEXWEB_DISCON:
#ifdef STM_DEBUG_MODE
            STM_ERR_PRINT("currnt state is[%d],"
                "but receive error evnet(%d)\n",
                _g_state_int, event_int);
#endif
            return eSTM_S_ITN_INIT_NON;
        default:
            return eSTM_S_ITN_ERROR;
    }
}

static int init_con_all_state(eStmEvent event_int)
{

    switch(event_int)
    {
        case eSTM_E_INIT_DONE:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_RECOV_ALL);
            //send_start_recovery();
            return eSTM_S_ITN_RECOV_ALL;
        case eSTM_E_DEINIT:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_DEINIT);
            return eSTM_S_ITN_DEINIT;
        case eSTM_E_ClOUD_DISCON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_INIT_FLX);
            return eSTM_S_ITN_INIT_FLX;
        case eSTM_E_FLEXWEB_DISCON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_INIT_CLD);
            return eSTM_S_ITN_INIT_CLD;
        case eSTM_E_INIT:
        case eSTM_E_SYNC:
        case eSTM_E_ClOUD_CON:
        case eSTM_E_FLEXWEB_CON:
#ifdef STM_DEBUG_MODE
            STM_ERR_PRINT("currnt state is[%d],"
                "but receive error evnet(%d)\n",
                _g_state_int, event_int);
#endif
            return eSTM_S_ITN_INIT_ALL;
        default:
            return eSTM_S_ITN_ERROR;
    }
}

static int init_con_cloud_state(eStmEvent event_int)
{

    switch(event_int)
    {
        case eSTM_E_INIT_DONE:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_RECOV_CLD);
            //send_start_recovery();
            return eSTM_S_ITN_RECOV_CLD;
        case eSTM_E_DEINIT:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_DEINIT);
            return eSTM_S_ITN_DEINIT;
        case eSTM_E_ClOUD_DISCON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_INIT_NON);
            return eSTM_S_ITN_INIT_NON;
        case eSTM_E_FLEXWEB_CON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_INIT_ALL);
            return eSTM_S_ITN_INIT_ALL;
        case eSTM_E_INIT:
        case eSTM_E_SYNC:
        case eSTM_E_ClOUD_CON:
        case eSTM_E_FLEXWEB_DISCON:
#ifdef STM_DEBUG_MODE
            STM_ERR_PRINT("currnt state is[%d],"
                "but receive error evnet(%d)\n",
                _g_state_int, event_int);
#endif
            return eSTM_S_ITN_INIT_CLD;
        default:
            return eSTM_S_ITN_ERROR;
    }
}

static int init_con_flexweb_state(eStmEvent event_int)
{

    switch(event_int)
    {
        case eSTM_E_INIT_DONE:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_RECOV_NON);
            //send_start_recovery();
            return eSTM_S_ITN_RECOV_NON;
        case eSTM_E_DEINIT:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_DEINIT);
            return eSTM_S_ITN_DEINIT;
        case eSTM_E_FLEXWEB_DISCON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_INIT_NON);
            return eSTM_S_ITN_INIT_NON;
        case eSTM_E_ClOUD_CON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_INIT_ALL);
            return eSTM_S_ITN_INIT_ALL;
        case eSTM_E_INIT:
        case eSTM_E_SYNC:
        case eSTM_E_FLEXWEB_CON:
        case eSTM_E_ClOUD_DISCON:
#ifdef STM_DEBUG_MODE
            STM_ERR_PRINT("currnt state is[%d],"
                "but receive error evnet(%d)\n",
                _g_state_int, event_int);
#endif
            return eSTM_S_ITN_INIT_FLX;
        default:
            return eSTM_S_ITN_ERROR;
    }
}


static int recovery_con_non_state(eStmEvent event_int)
{
    switch(event_int)
    {
        case eSTM_E_SYNC:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_LOCAL_NON);
            return eSTM_S_ITN_LOCAL_NON;
            break;
        case eSTM_E_ClOUD_CON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_RECOV_CLD);
            return eSTM_S_ITN_RECOV_CLD;
        case eSTM_E_FLEXWEB_CON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_RECOV_FLX);
            return eSTM_S_ITN_RECOV_FLX;
        case eSTM_E_DEINIT:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_DEINIT);
            return eSTM_S_ITN_DEINIT;
            break;
        case eSTM_E_INIT:
        case eSTM_E_INIT_DONE:
        case eSTM_E_ClOUD_DISCON:
        case eSTM_E_FLEXWEB_DISCON:
#ifdef STM_DEBUG_MODE
            STM_ERR_PRINT("currnt state is[%d],"
                "but receive error evnet(%d)\n",
                _g_state_int, event_int);
#endif
            return eSTM_S_ITN_RECOV_ALL;
        default:
            return eSTM_S_ITN_ERROR;
    }

}

static int recovery_con_all_state(eStmEvent event_int)
{
    switch(event_int)
    {
        case eSTM_E_SYNC:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_ClOUD);
            return eSTM_S_ITN_ClOUD;
        case eSTM_E_ClOUD_DISCON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_RECOV_FLX);
            return eSTM_S_ITN_RECOV_FLX;
        case eSTM_E_FLEXWEB_DISCON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_RECOV_CLD);
            return eSTM_S_ITN_RECOV_CLD;
        case eSTM_E_DEINIT:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_DEINIT);
            return eSTM_S_ITN_DEINIT;
        case eSTM_E_INIT:
        case eSTM_E_INIT_DONE:
        case eSTM_E_ClOUD_CON:
        case eSTM_E_FLEXWEB_CON:
#ifdef STM_DEBUG_MODE
            STM_ERR_PRINT("currnt state is[%d],"
                "but receive error evnet(%d)\n",
                _g_state_int, event_int);
#endif
            return eSTM_S_ITN_RECOV_ALL;
        default:
            return eSTM_S_ITN_ERROR;
    }
}

static int recovery_con_cloud_state(eStmEvent event_int)
{
    switch(event_int)
    {
        case eSTM_E_SYNC:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_LOCAL_CLD);
            return eSTM_S_ITN_LOCAL_CLD;
        case eSTM_E_FLEXWEB_CON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_RECOV_ALL);
            //send_start_recovery();
            return eSTM_S_ITN_RECOV_ALL;
        case eSTM_E_ClOUD_DISCON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_RECOV_NON);
            return eSTM_S_ITN_RECOV_NON;
        case eSTM_E_DEINIT:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_DEINIT);
            return eSTM_S_ITN_DEINIT;
        case eSTM_E_INIT:
        case eSTM_E_INIT_DONE:
        case eSTM_E_ClOUD_CON:
        case eSTM_E_FLEXWEB_DISCON:
#ifdef STM_DEBUG_MODE
            STM_ERR_PRINT("currnt state is[%d],"
                "but receive error evnet(%d)\n",
                _g_state_int, event_int);
#endif
            return eSTM_S_ITN_RECOV_CLD;
        default:
            return eSTM_S_ITN_ERROR;
    }
}

static int recovery_con_flexweb_state(eStmEvent event_int)
{
    switch(event_int)
    {
        case eSTM_E_SYNC:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_LOCAL_CLD);
            return eSTM_S_ITN_LOCAL_CLD;
        case eSTM_E_ClOUD_CON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_RECOV_ALL);
            //send_start_recovery();
            return eSTM_S_ITN_RECOV_ALL;
        case eSTM_E_FLEXWEB_DISCON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_RECOV_NON);
            return eSTM_S_ITN_RECOV_NON;
        case eSTM_E_DEINIT:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_DEINIT);
            return eSTM_S_ITN_DEINIT;
        case eSTM_E_INIT:
        case eSTM_E_INIT_DONE:
        case eSTM_E_FLEXWEB_CON:
        case eSTM_E_ClOUD_DISCON:
#ifdef STM_DEBUG_MODE
            STM_ERR_PRINT("currnt state is[%d],"
                "but receive error evnet(%d)\n",
                _g_state_int, event_int);
#endif
            return eSTM_S_ITN_RECOV_CLD;
        default:
            return eSTM_S_ITN_ERROR;
    }
}

static int cloud_state(eStmEvent event_int)
{
    switch(event_int)
    {
        case eSTM_E_ClOUD_CON:
            return eSTM_S_ITN_ClOUD;
        case eSTM_E_FLEXWEB_CON:
            return eSTM_S_ITN_ClOUD;
        case eSTM_E_ClOUD_DISCON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_LOCAL_FLX);
            return eSTM_S_ITN_LOCAL_FLX;
        case eSTM_E_FLEXWEB_DISCON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_LOCAL_CLD);
            return eSTM_S_ITN_LOCAL_CLD;
        case eSTM_E_DEINIT:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_DEINIT);
            return eSTM_S_ITN_DEINIT;
        case eSTM_E_INIT:
        case eSTM_E_INIT_DONE:
        case eSTM_E_SYNC:
#ifdef STM_DEBUG_MODE
            STM_ERR_PRINT("currnt state is[%d],"
                "but receive error evnet(%d)\n",
                _g_state_int, event_int);
#endif
            return eSTM_S_ITN_ClOUD;
        default:
            return eSTM_S_ITN_ERROR;
    }
}

static int local_con_non_state(eStmEvent event_int)
{
    switch(event_int)
    {
        case eSTM_E_ClOUD_CON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_LOCAL_CLD);
            return eSTM_S_ITN_LOCAL_CLD;
        case eSTM_E_FLEXWEB_CON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_LOCAL_FLX);
            return eSTM_S_ITN_LOCAL_FLX;
        case eSTM_E_DEINIT:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_DEINIT);
            return eSTM_S_ITN_DEINIT;
        case eSTM_E_INIT:
        case eSTM_E_INIT_DONE:
        case eSTM_E_SYNC:
        case eSTM_E_FLEXWEB_DISCON:
        case eSTM_E_ClOUD_DISCON:
#ifdef STM_DEBUG_MODE
            STM_ERR_PRINT("currnt state is[%d],"
                "but receive error evnet(%d)\n",
                _g_state_int, event_int);
#endif
            return eSTM_S_ITN_LOCAL_NON;
        default:
            return eSTM_S_ITN_ERROR;
    }
}

static int local_con_cloud_state(eStmEvent event_int)
{
    switch(event_int)
    {
        case eSTM_E_FLEXWEB_CON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_RECOV_ALL);
            //send_start_recovery();
            return eSTM_S_ITN_RECOV_ALL;
        case eSTM_E_ClOUD_DISCON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_LOCAL_NON);
            return eSTM_S_ITN_LOCAL_NON;
        case eSTM_E_DEINIT:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_DEINIT);
            return eSTM_S_ITN_DEINIT;
        case eSTM_E_INIT:
        case eSTM_E_INIT_DONE:
        case eSTM_E_SYNC:
        case eSTM_E_FLEXWEB_DISCON:
        case eSTM_E_ClOUD_CON:
#ifdef STM_DEBUG_MODE
            STM_ERR_PRINT("currnt state is[%d],"
                "but receive error evnet(%d)\n",
                _g_state_int, event_int);
#endif
            return eSTM_S_ITN_LOCAL_CLD;
        default:
            return eSTM_S_ITN_ERROR;
    }

}

static int local_con_flexweb_state(eStmEvent event_int)
{
    switch(event_int)
    {
        case eSTM_E_ClOUD_CON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_RECOV_ALL);
            //send_start_recovery();
            return eSTM_S_ITN_RECOV_ALL;
        case eSTM_E_FLEXWEB_DISCON:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_LOCAL_NON);
            return eSTM_S_ITN_LOCAL_NON;
            break;
        case eSTM_E_DEINIT:
            __sync_lock_test_and_set(&_g_state_int, eSTM_S_ITN_DEINIT);
            return eSTM_S_ITN_DEINIT;
            break;
        case eSTM_E_INIT:
        case eSTM_E_INIT_DONE:
        case eSTM_E_SYNC:
        case eSTM_E_FLEXWEB_CON:
        case eSTM_E_ClOUD_DISCON:
#ifdef STM_DEBUG_MODE
            STM_ERR_PRINT("currnt state is[%d],"
                "but receive error evnet(%d)\n",
                _g_state_int, event_int);
#endif
            return eSTM_S_ITN_LOCAL_FLX;
            break;
        default:
            return eSTM_S_ITN_ERROR;
            break;
    }
}


static int deinit_state(eStmEvent event_int)
{
    switch(event_int)
    {
        case eSTM_E_INIT:
            return eSTM_S_ITN_DEINIT;
            break;
        case eSTM_E_INIT_DONE:
            return eSTM_S_ITN_DEINIT;
            break;
        case eSTM_E_SYNC:
            return eSTM_S_ITN_DEINIT;
            break;
        case eSTM_E_ClOUD_CON:
            return eSTM_S_ITN_DEINIT;
            break;
        case eSTM_E_FLEXWEB_CON:
            return eSTM_S_ITN_DEINIT;
            break;
        case eSTM_E_ClOUD_DISCON:
            return eSTM_S_ITN_DEINIT;
            break;
        case eSTM_E_FLEXWEB_DISCON:
            return eSTM_S_ITN_DEINIT;
            break;
        case eSTM_E_DEINIT:
            return eSTM_S_ITN_DEINIT;
            break;
        default:
            return eSTM_S_ITN_ERROR;
            break;
    }
}
/*
static int error_state(eStmEvent event_int)
{
    return eSTM_S_ITN_ERROR;
}
*/
