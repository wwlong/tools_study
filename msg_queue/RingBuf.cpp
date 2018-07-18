/*
	RingBuf

	To buffer frames of RGB or depth stream
*/
#include "RingBuf.h"
#include "stdlib.h"


RingBuf::RingBuf(int sz)
{
	pthread_mutexattr_t attr;
	
	ppbuf = (void **)malloc(sizeof(void *) * sz);
	size = sz;
	count = rd_idx = wr_idx = 0; 
	
	pthread_mutex_init(&mut, &attr);
}


RingBuf::~RingBuf()
{
	void * pdata = popFrame();
	while (pdata)
	{
		free(pdata);
		pdata = popFrame();
	}
	
	pthread_mutex_destroy(&mut);
	free(ppbuf);
}

//0: OK , -1 fail(buffer full)
int RingBuf::pushAllocatedFrame(void *pframe)
{
	if (count >= size)	return -1;
	
	pthread_mutex_lock(&mut);
	ppbuf[wr_idx++] = pframe;
	
	if (wr_idx >= size) wr_idx = 0; 
	count ++;
	pthread_mutex_unlock(&mut);
    return 0;
}


void *RingBuf::popFrame()
{
	void * pret = NULL; 
	if (count == 0) return NULL;
	
	pthread_mutex_lock(&mut);
	pret = ppbuf[rd_idx++];
	
	if (rd_idx >= size) rd_idx = 0; 
	count --;
	pthread_mutex_unlock(&mut);
	return pret;
}

void RingBuf::cleanup()
{
	void * pdata = popFrame();
	while (pdata)
	{
		free(pdata);
		pdata = popFrame();
	}
}
