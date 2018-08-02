#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "msgq.h"
//#include "common_api.h"
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
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
LP_LIST msgqInit(int32_t maxLength){ 
    if(maxLength < 0) {
        printf("invaild args\n");
        return NULL;
    }
    LP_LIST list = (LP_LIST)malloc(sizeof(ST_LIST));
    if(NULL == list) {
        printf("list malloc failed\n");
        return NULL;
    }
    memset(list, 0, sizeof(ST_LIST));
    list->MAX_LENGTH = maxLength;
    list->length = 0;
	INIT_LIST_HEAD(&(list->list_node.list));
    return list;
}

/**
* @brief
* @param
* @retval
*/
int32_t msgqSet(LP_LIST list, void *ptr) {
    if(NULL == list || NULL == ptr) {
        printf("invalid params\n");
        return -1;
    }
    pthread_mutex_lock(&mutex);
    if(list->length >= list->MAX_LENGTH) {
        printf("msgq full\n");
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    LP_LIST_NODE tmp = (LP_LIST_NODE)malloc(sizeof(ST_LIST_NODE));
    if(NULL == tmp) {
        printf("list_node malloc failed\n");
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    memset(tmp, 0, sizeof(ST_LIST_NODE));
//    tmp->ptr = ptr;
        memcpy(tmp->ptr, ptr, sizeof(void*));
        list->length ++;
        list_add_tail(&(tmp->list), &(list->list_node.list));
    pthread_mutex_unlock(&mutex);
    return 0;
}
/**
* @brief
* @param
* @retval
*/
int32_t msgqGet(LP_LIST list, void *ptr) {
    if(NULL == list || NULL == ptr) {
        printf("invalid params\n");
        return -1;
    }
    pthread_mutex_lock(&mutex);
	LP_LIST_NODE tmp; 
	struct list_head *pos, *q;
    if(list->length <= 0) {
        printf("empty queue\n");
        pthread_mutex_unlock(&mutex);
        return -1;
    }
	list_for_each_safe(pos, q, &(list->list_node.list)){
		 tmp = list_entry(pos, ST_LIST_NODE, list);
//		 ptr = tmp->ptr;
         if(tmp != NULL) {
            memcpy(ptr, tmp->ptr, sizeof(void*));
            //ptr = tmp->ptr;
            list_del(pos);
            free(tmp);
            list->length --;
            break;
         }
	}
    pthread_mutex_unlock(&mutex);
    return 0;
}
/**
* @brief
* @param
* @retval
*/
int32_t msgqDestroy(LP_LIST list) {
    if(NULL == list) {
        printf("NULL args\n");
        return -1;
    }
	LP_LIST_NODE tmp; 
	struct list_head *pos, *q;
    pthread_mutex_lock(&mutex);
	list_for_each_safe(pos, q, &(list->list_node.list)){
		 tmp = list_entry(pos, ST_LIST_NODE, list);
         list_del(pos);
		 free(tmp);
         list->length --;
	}
    pthread_mutex_unlock(&mutex);
    free(list);
    list = NULL;
    return 0;
}
