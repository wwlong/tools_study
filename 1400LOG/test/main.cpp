#include <stdio.h>
#include <string.h>
#include "1400log.h"


int main()
{
    const char *logname = "log.txt";
    LP_DG_LOG logHandler = log_init(logname);
    if(NULL == logHandler) {
        printf("logHandler init failed\n");
        return -1;
    }

    log_enable(logHandler);

    int count = 0;
    while(1) {
        char msg[1024];
        memset(msg, 0, sizeof(msg));
        sprintf(msg, "hello_%d", count ++);
        log_save(logHandler, msg);
        count ++;
        if(count > 100) {
            break;
        }
    }

    //清理资源
    log_deinit(logHandler);
    return 0;
}