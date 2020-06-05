/********************************************************
 * Filename: core/task.c
 * 
 * Author: parkjy, RTOSLab. SNU.
 * 
 * Description: task management.
 ********************************************************/
#include <core/eos.h>

#define READY		1
#define RUNNING		2
#define WAITING		3

/*
 * Queue (list) of tasks that are ready to run.
 */
static _os_node_t *_os_ready_queue[LOWEST_PRIORITY + 1];

/*
 * Pointer to TCB of running task
 */
static eos_tcb_t *_os_current_task;

int32u_t eos_create_task(eos_tcb_t *task, addr_t sblock_start, size_t sblock_size, void (*entry)(void *arg), void *arg, int32u_t priority) {
	PRINT("task: 0x%x, priority: %d\n", (int32u_t)task, priority);

	// fill out tcb
	task->sp = _os_create_context(sblock_start, sblock_size, entry, arg);
	task->priority = priority;

	// make new node with tcb, and push on ready queue
	_os_node_t *new_node = malloc(sizeof(_os_node_t));
	new_node->ptr_data = (void *)task;
	new_node->priority = priority;
	_os_add_node_tail(&(_os_ready_queue[priority]), new_node);

	// acknowledge node
	_os_set_ready(priority);
}

int32u_t eos_destroy_task(eos_tcb_t *task) {
}

void eos_schedule() {
	if(_os_current_task != NULL)
	{
		//PRINT("saving context of task: %p \n", _os_current_task);
		// if there is task running, save task, update tcb
		addr_t sp = _os_save_context();
		if(sp == NULL) return;

		_os_current_task->sp = sp;
	}

	// get highest priority
	int32u_t priority = _os_get_highest_priority();

	// remove from ready queue
	_os_node_t *node = _os_ready_queue[priority];
	_os_current_task = (eos_tcb_t*)(node->ptr_data);
	//PRINT("ready to run task: %p \n", _os_current_task);
	_os_remove_node(&(_os_ready_queue[priority]), node);
	free(node);
	
	// remove from ready list
	_os_unset_ready(priority);

	// restore context and run
	_os_restore_context(_os_current_task->sp);
}

eos_tcb_t *eos_get_current_task() {
	return _os_current_task;
}

void eos_change_priority(eos_tcb_t *task, int32u_t priority) {
}

int32u_t eos_get_priority(eos_tcb_t *task) {
}

void eos_set_period(eos_tcb_t *task, int32u_t period){
	task->period = period;
}

int32u_t eos_get_period(eos_tcb_t *task) {
}

int32u_t eos_suspend_task(eos_tcb_t *task) {
}

int32u_t eos_resume_task(eos_tcb_t *task) {
}

void eos_sleep(int32u_t tick) {
	//make alarm
	eos_alarm_t *new_alarm = malloc(sizeof(eos_alarm_t));

	eos_set_alarm(eos_get_system_timer(), new_alarm, _os_current_task->period, _os_wakeup_sleeping_task, _os_current_task);

	// if done, schedule
	eos_schedule();
}

void _os_init_task() {
	PRINT("initializing task module.\n");

	/* init current_task */
	_os_current_task = NULL;

	/* init multi-level ready_queue */
	int32u_t i;
	for (i = 0; i < LOWEST_PRIORITY; i++) {
		_os_ready_queue[i] = NULL;
	}
}

void _os_wait(_os_node_t **wait_queue) {
}

void _os_wakeup_single(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_all(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_sleeping_task(void *arg) {
	// make node with tcb and push on ready queue
	_os_node_t *new_node = malloc(sizeof(_os_node_t));
	new_node->ptr_data = arg;
	new_node->priority = ((eos_tcb_t*)arg)->priority;
	_os_add_node_tail(&(_os_ready_queue[new_node->priority]), new_node);
	_os_set_ready(new_node->priority);
}
