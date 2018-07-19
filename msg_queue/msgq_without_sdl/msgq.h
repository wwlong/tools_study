#ifndef MSGQ_H
#define MSGQ_H

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if defined __cplusplus
extern "C" {
#endif



typedef struct MSGQ_S {
	uint8_t * queue;
	uint32_t msg_size;
	int max_msg;
	int write;
	int read;
	int nmsg;
	sem_t signal;
	pthread_mutex_t guard;
}MSGQ_T;

MSGQ_T *MSGQ_Create(uint32_t msg_size,uint32_t max_msg	);
void MSGQ_Destroy(	MSGQ_T * handle	);
int MSGQ_Post(	MSGQ_T * hndl,	void * msg	);
int MSGQ_Wait(	MSGQ_T* hndl,	void * msg	);
int MSGQ_Flush(	MSGQ_T *hndl	);
int MSGQ_WaitTimeout(	MSGQ_T* hndl,	void * msg, int to);
int MSGQ_TryWait(	MSGQ_T* hndl,	void * msg);
int MSGQ_Peep(	MSGQ_T* hndl,	void * msg	);


#if defined __cplusplus
}
#endif

#endif
