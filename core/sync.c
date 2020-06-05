/********************************************************
 * Filename: core/sync.c
 * 
 * Author: wsyoo, RTOSLab. SNU.
 * 
 * Description: semaphore, condition variable management.
 ********************************************************/
#include <core/eos.h>

void eos_init_semaphore(eos_semaphore_t *sem, int32u_t initial_count, int8u_t queue_type) {
	sem->count = initial_count;
	sem->wait_queue = NULL;
	sem->queue_type = queue_type;
}

int32u_t eos_acquire_semaphore(eos_semaphore_t *sem, int32s_t timeout) {
	int32u_t flag = eos_disable_interrupt();

	while(1) {
		// resource available
		if(sem->count > 0) {
			sem->count--;
			eos_restore_interrupt(flag);

			extern eos_mqueue_t mq1;
			extern eos_mqueue_t mq2;

			if(sem == &(mq1.getsem)) {
				PRINT("%-15s%33s\n", "mq1 [receive]", "success");
			} else if(sem == &(mq2.getsem)) {
				PRINT("%-15s%33s\n", "mq2 [receive]", "success");
			} else if(sem == &(mq1.putsem)) {
				PRINT("%-15s%33s\n", "mq1 [send]", "success");
			} else if(sem == &(mq2.putsem)) {
				PRINT("%-15s%33s\n", "mq2 [send]", "success");
			}

			return 1;
		} else {
			// not available

			extern eos_mqueue_t mq1;
			extern eos_mqueue_t mq2;

			if(sem == &(mq1.getsem)) {
				PRINT("%-15s%33s\n", "mq1 [receive]", "failed");
			} else if(sem == &(mq2.getsem)) {
				PRINT("%-15s%33s\n", "mq2 [receive]", "failed");
			} else if(sem == &(mq1.putsem)) {
				PRINT("%-15s%33s\n", "mq1 [send]", "failed");
			} else if(sem == &(mq2.putsem)) {
				PRINT("%-15s%33s\n", "mq2 [send]", "failed");
			}

			if(timeout == -1) {
				// can't wait

				eos_restore_interrupt(flag);
				return 0;

			} else if(timeout == 0) {
				// goto ready queue immediately

				_os_node_t *new_node = malloc(sizeof(_os_node_t));
				new_node->ptr_data = (void *)eos_get_current_task();
				new_node->priority = eos_get_current_task()->priority;

				if(sem->queue_type) 
					_os_add_node_priority(&(sem->wait_queue), new_node);
				else
					_os_add_node_tail(&(sem->wait_queue), new_node);

				eos_schedule();

			} else if(timeout > 0) {
				// wait (timeout) secs until being ready
				//
				extern system_timer;
				eos_alarm_t *alarm = malloc(sizeof(eos_alarm_t));
				eos_set_alarm(eos_get_system_timer(), alarm, timeout, _os_wakeup_sleeping_task, eos_get_current_task());
				eos_schedule();
				// if returned after sleep, make timeout zero and be ready
				timeout = 0;

			}
		}
	}
	


	eos_restore_interrupt(flag);
}

void eos_release_semaphore(eos_semaphore_t *sem) {
	int32u_t flag = eos_disable_interrupt();

	sem->count++;
	// if there is a task waiting for semaphore...
	if(sem->wait_queue) {
		_os_wakeup_sleeping_task(sem->wait_queue->ptr_data);
		_os_node_t *node = sem->wait_queue;
		_os_remove_node(&(sem->wait_queue), node);
		free(node);
	}

	eos_restore_interrupt(flag);
}

void eos_init_condition(eos_condition_t *cond, int32u_t queue_type) {
	/* initialization */
	cond->wait_queue = NULL;
	cond->queue_type = queue_type;
}

void eos_wait_condition(eos_condition_t *cond, eos_semaphore_t *mutex) {
	/* release acquired semaphore */
	eos_release_semaphore(mutex);
	/* wait on condition's wait_queue */
	_os_wait(&cond->wait_queue);
	/* acquire semaphore before return */
	eos_acquire_semaphore(mutex, 0);
}

void eos_notify_condition(eos_condition_t *cond) {
	/* select a task that is waiting on this wait_queue */
	_os_wakeup_single(&cond->wait_queue, cond->queue_type);
}
