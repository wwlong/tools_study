#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/*
 *  测试msgq
 * */
#include "msgq.h"
void *taska(void *args)
{
    LP_LIST list = (LP_LIST)args;
    if(NULL == list) {
        printf("list NULL\n");
        return NULL;
    }
    printf("list handler : %p\n", list);
    char *msg = NULL;
    int cnt = 0;
    while(1) {
        msg = (char *)malloc(64);
        if(NULL == msg) {
            printf("msg NULL\n");
            continue;
        }
        memset(msg, 0, sizeof(64));
        sprintf(msg, "hello%d=========", cnt ++);
        if(0 != msgqSet(list, &msg)) {
            free(msg);
            msg=NULL;
        }
//        usleep(100);
    }
}

void *taskb(void *args)
{
    LP_LIST list = (LP_LIST)args;
    if(NULL == list) {
        printf("list NULL\n");
        return NULL;
    }
    printf("bbbbbb list handler : %p\n", list);
    char *msg = NULL;
    int cnt = 0;
    while(1) {
        msgqGet(list, &msg);
        if(NULL != msg) {
            printf("msg : %s\n", msg);
            free(msg);
            msg = NULL;
        }
        else {
            //printf("msg NULL\n");
        }
//        usleep(100);
    }
}

int main()
{
    LP_LIST msgq_handler = msgqInit(5);
    LP_LIST list = msgq_handler; 
    int cnt = 0;
    char *msg = (char *)malloc(64);
    memset(msg, 0, sizeof(64));
    sprintf(msg, "hello%d", cnt ++);
    if(0 != msgqSet(list, &msg)) {
        free(msg);
    }
    msg = NULL;
    printf("list handler : %p\n", list);
    msgqGet(list, &msg);
    if(NULL != msg) {
        printf("msg : %s\n", msg);
        free(msg);
    }

//    while(1) {
//        sleep(1);
//    }
    pthread_t taska_t, taskb_t;
    pthread_create(&taska_t, NULL, taska, msgq_handler);
    pthread_create(&taskb_t, NULL, taskb, msgq_handler);

   pthread_join(taska_t, NULL);
   pthread_join(taskb_t, NULL);
    return 0;
}
