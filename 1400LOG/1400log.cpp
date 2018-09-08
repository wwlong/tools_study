/*
    保存log到文件
*/
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "1400log.h"
/*
    init
    enable 
    diable
    save
    deinit
*/
LP_DG_LOG log_init(const char *filename){
    if(NULL == filename) {
        printf("log_init filename NULL\n");
        return NULL;
    }
    LP_DG_LOG tmp = (LP_DG_LOG)malloc(sizeof(ST_DG_LOG));
    if(NULL == tmp) {
        printf("log_init malloc failed\n");
        return NULL;
    }
    memset(tmp, 0, sizeof(ST_DG_LOG));
    if(strlen(filename) > FILENAME_MAX_SIZE) {
        printf("filename too long,must shorter than %d\n", FILENAME_MAX_SIZE);
        return NULL;
    }
    memcpy(tmp->filename, filename, strlen(filename));
    tmp->fp = fopen(filename, "a");
    if(NULL == tmp->fp) {
        printf("%s open failed\n", filename);
        printf("%s -- %d\n", strerror(errno), errno);
        return NULL;
    }
    return tmp;
}

int log_enable(LP_DG_LOG logHandler) {
     if(NULL == logHandler) {
        printf("invaild params NULL\n");
        return -1;
    }
    logHandler->enable = true;
}

int log_disable(LP_DG_LOG logHandler) {
    if(NULL == logHandler) {
        printf("invaild params NULL\n");
        return -1;
    }
    logHandler->enable = false;
}

int log_save(LP_DG_LOG logHandler, const char *msg) {
    if(NULL == logHandler || msg == NULL) {
        printf("invaild params NULL\n");
        return -1;
    }
    if(false == logHandler->enable) {
        printf("log diabled\n");
        return -2;
    }
    //写入log
    char buffer[4096] = {0};		
    va_list args;
    int len = 0;
    time_t currtime;
    time(&currtime);
    struct tm tm_ymd;
    localtime_r(&currtime,&tm_ymd);
    len = snprintf(buffer, sizeof(buffer), "%4d-%02d-%02d-%02d-%02d-%02d: ", tm_ymd.tm_year + 1900, tm_ymd.tm_mon + 1, tm_ymd.tm_mday, tm_ymd.tm_hour, tm_ymd.tm_min, tm_ymd.tm_sec);
    fwrite(buffer, len, 1, logHandler->fp);
    fwrite(msg, strlen(msg), 1, logHandler->fp);
    fwrite("\n", 1, 1, logHandler->fp);
    fflush(logHandler->fp);

    return 0;
}
/*
    清理资源
*/
int log_deinit(LP_DG_LOG logHandler) {
    if(NULL == logHandler) {
        printf("invaild params NULL\n");
        return -1;
    }
    if(NULL != logHandler->fp) {
        fclose(logHandler->fp);
        logHandler->fp = NULL;
    }
    free(logHandler);
    return 0;
}