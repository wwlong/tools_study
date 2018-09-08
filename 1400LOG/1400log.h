#ifndef LOG1400_H_
#define LOG1400_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#define FILENAME_MAX_SIZE 128
#define SINGLE_LOG_MAX_LENGTH (1024*10)
typedef struct {
    FILE *fp;
    char filename[FILENAME_MAX_SIZE];
    bool enable;
}ST_DG_LOG, *LP_DG_LOG;
/*
    init
    enable 
    diable
    save
    deinit
*/
LP_DG_LOG log_init(const char *filename);

int log_enable(LP_DG_LOG logHandler);

int log_disable(LP_DG_LOG logHandler);

int log_save(LP_DG_LOG logHandler, const char *msg);
/*
    清理资源
*/
int log_deinit(LP_DG_LOG logHandler);
#ifdef __cplusplus
}
#endif

#endif