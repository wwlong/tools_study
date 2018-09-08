/**********************************************************
 * Author        : wenlong
 * Company       : deepglint
 * Last modified : 2018-09-08 13:35
 * Filename      : time_demo.c
 * Description   : 
 * *******************************************************/
#include <stdio.h>
#include <time.h>
int main()
{
    time_t currtime;
    time(&currtime);
    struct tm tm_ymd;
    localtime_r(&currtime,&tm_ymd);
    char cur_time[64] = {0};
    snprintf(cur_time, sizeof(cur_time), "%04d-%02d-%02d %02d:%02d:%02d", tm_ymd.tm_year + 1900, tm_ymd.tm_mon + 1, tm_ymd.tm_mday, tm_ymd.tm_hour, tm_ymd.tm_min, tm_ymd.tm_sec);
    
    printf("currtime : %s\n", cur_time);

    return 0;
}
