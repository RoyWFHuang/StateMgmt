
#include <pthread.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <unistd.h>
#include "state_mgmt.h"

int send_start_recovery()
{
    return 1;
}

void* cloud_sync_func(void *arg)
{
    tStmEventMsg stmeve_struct;
    int qid_int = msgget(STATE_MGMT_QUEUE_ID, IPC_CREAT|0666);
    while(1)
    {
        stmeve_struct.event_int = eSTM_E_ClOUD_CON;
        int ret_int = msgsnd(qid_int,
            &stmeve_struct,
            sizeof(tStmEventMsg)-sizeof(long),
            0
        );
        sleep(5);
    }
}

void* flex_sync_func(void *arg)
{
    tStmEventMsg stmeve_struct;
    int qid_int = msgget(STATE_MGMT_QUEUE_ID, IPC_CREAT|0666);
    while(1)
    {
        stmeve_struct.event_int = eSTM_E_FLEXWEB_CON;
        int ret_int = msgsnd(qid_int,
            &stmeve_struct,
            sizeof(tStmEventMsg)-sizeof(long),
            0
        );
        sleep(8);
    }
}


int main()
{
    initial_state_mgmt();
/*
    pthread_t cloud_sync;
    pthread_t flex_sync;

    pthread_create(&cloud_sync,
        NULL, cloud_sync_func, NULL);
    pthread_create(&flex_sync,
        NULL, flex_sync_func, NULL);
*/
    tStmEventMsg stmeve_struct;
    int qid_int = msgget(STATE_MGMT_QUEUE_ID, IPC_CREAT|0666);
    stmeve_struct.type_long = STM_RECV_TYPE;
    while(1)
    {
        int event_int;
        scanf("%d", &event_int);
        if (eSTM_E_END <= event_int)
            break;
        stmeve_struct.event_int = event_int;
        int ret_int = msgsnd(qid_int,
            &stmeve_struct,
            sizeof(tStmEventMsg)-sizeof(long),
            0
        );
    }
    deinit_state_mgmt();
    return 0;
}