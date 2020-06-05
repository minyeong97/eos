/********************************************************
 * Filename: core/comm.c
 *  
 * Author: jtlim, RTOSLab. SNU.
 * 
 * Description: message queue management. 
 ********************************************************/
#include <core/eos.h>

void eos_init_mqueue(eos_mqueue_t *mq, void *queue_start, int16u_t queue_size, int8u_t msg_size, int8u_t queue_type) {
	mq->queue_size = queue_size;
	mq->msg_size = msg_size;
	mq->queue_start = queue_start;
	mq->front = queue_start;
	mq->rear = queue_start;
	mq->putsem.count = queue_size;
	mq->putsem.queue_type = queue_type;
	mq->getsem.count = 0;
	mq->getsem.queue_type = queue_type;
}

int8u_t eos_send_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
	int32u_t res = eos_acquire_semaphore(&(mq->putsem), timeout);

	if(res == 0) {
		// if failed, return if timeout is -1
		if(timeout == -1) return;
	}

	// cannot use strncpy
	for(int i = 0; i < mq->msg_size; i++) {
		int8u_t msg = *((int8u_t*)message + i);
		int8u_t *cur = (int8u_t*)(mq->rear);

		*cur = msg;

		mq->rear = cur + 1;
		if ((int8u_t*)(mq->rear) == ((int8u_t*)(mq->queue_start) + mq->queue_size*mq->msg_size))
			mq->rear = mq->queue_start;
		
	}

	eos_release_semaphore(&(mq->getsem));
}

int8u_t eos_receive_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
	int32u_t res = eos_acquire_semaphore(&(mq->getsem), timeout);


	if(res == 0) {
		if(timeout == -1) return;
	}

	// cannot use strncpy
	for(int i = 0; i < mq->msg_size; i++) {
		int8u_t *msg = (int8u_t*)message;
		int8u_t *cur = (int8u_t*)(mq->front);
		
		*(msg + i) = *cur;

		mq->front = cur + 1;
		if ((int8u_t*)(mq->front) == ((int8u_t*)(mq->queue_start) + mq->queue_size*mq->msg_size))
			mq->front = mq->queue_start;
	}

	eos_release_semaphore(&(mq->putsem));

}
