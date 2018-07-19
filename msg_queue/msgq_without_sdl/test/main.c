#include <stdio.h>

#include "msgq.h"

int main()
{
    MSGQ_T *msgq_handler = MSGQ_Create(4, 100);
    if(NULL == msgq_handler) {
        printf("MSGQ_Create failed\n");
        return -1;
    }
    int i = 0;
    for(i = 0;i < 10;i ++) {
        MSGQ_Post(msgq_handler, &i);
    }

    for(i = 0;i < 10;i ++) {
        int temp = 0;
        MSGQ_Wait(msgq_handler, &temp);
        printf("%d\n", temp);
    }

    return 0;
}
