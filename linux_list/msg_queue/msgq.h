
#ifndef MSGQ_H_
#define MSGQ_H_ 

#ifdef __cplusplus
extern "C"
{
#endif 
#include <stdint.h>
#include "list.h"

/*
 *  基于list list的message queue 
 * */ 
typedef struct {
   	struct list_head list;
    char ptr[4];
}ST_LIST_NODE,*LP_LIST_NODE;

typedef struct {
    int32_t MAX_LENGTH;
    int32_t length;
//    pthread_mutex_t lock;
    ST_LIST_NODE list_node;
}ST_LIST, *LP_LIST;

/*
 *  init 
 *  set
 *  get 
 *  tryread
 *  destroy 
 * */
/*
 *  目前只支持传输指针
 * */

/**
* @brief
* @param
* @retval
*/

/**
* @brief    list init 
* @param
* @retval
*/
LP_LIST msgqInit(int32_t maxLength);

/**
* @brief
* @param
* @retval
*/
int32_t msgqSet(LP_LIST list, void *ptr);
/**
* @brief
* @param
* @retval
*/
int32_t msgqGet(LP_LIST list, void *ptr);
/**
* @brief
* @param
* @retval
*/
int32_t msgqDestroy(LP_LIST list);
/**
* @brief
* @param
* @retval
*/
#ifdef __cplusplus
}
#endif
#endif
