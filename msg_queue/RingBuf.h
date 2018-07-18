
#ifndef RINGBUF_H
#define RINGBUF_H

#include <pthread.h>

class RingBuf 
{
public:
	RingBuf(int sz);
	~RingBuf();
	
	int pushAllocatedFrame(void *pframe);//pframe must be allocated outside by caller
	void *popFrame();		//caller need to free
	int getBufferCount() { return count; }
	void cleanup();//clear/free buffer 
private:
	void **ppbuf;
	int size;
	int count;
	int rd_idx;
	int wr_idx; 
	
	pthread_mutex_t mut;	
};

#endif
