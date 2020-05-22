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
	task->sp = _os_create_context(sblock_start, sblock_size, entry, arg);
	task->priority = priority;
	_os_node_t *new_node = malloc(sizeof(_os_node_t));
	new_node->ptr_data = (void *)task;
	new_node->priority = priority;
	_os_add_node_tail(&(_os_ready_queue[priority]), new_node);
}

int32u_t eos_destroy_task(eos_tcb_t *task) {
}

void eos_schedule() {
	if(_os_current_task != NULL)
	{
		addr_t sp = _os_save_context();
		if(sp == NULL) return;

		_os_current_task->sp = sp;
		_os_node_t *new_node = malloc(sizeof(_os_node_t));
		new_node->ptr_data = (void *)_os_current_task;
		new_node->priority = _os_current_task->priority;
		_os_add_node_tail(&(_os_ready_queue[_os_current_task->priority]), new_node);
	}

	int32u_t priority = _os_get_highest_priority();
	_os_node_t *node = _os_ready_queue[priority];
	_os_current_task = (eos_tcb_t*)(node->ptr_data);
	_os_remove_node(_os_ready_queue, node);
	free(node);
		
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
	extern system_timer;
	eos_alarm_t *new_alarm = malloc(sizeof(struct eos_alarm_t));

	eos_set_alarm(&system_timer, new_alarm, system_timer->tick+_os_current_task->period, _os_wakeup_sleeping_task(), _os_current_task);

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
	eos_tcb_t *task = arg;
	_os_node_t *new_node = malloc(sizeof(_os_node_t));
	new_node->ptr_data = (void *)task;
	new_node->priority = task->priority;
	_os_add_node_tail(&(_os_ready_queue[task->priority]), new_node);
	_os_set_ready((task->priority) & 0xFF);
}
