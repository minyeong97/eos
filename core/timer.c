/********************************************************
 * Filename: core/timer.c
 *
 * Author: wsyoo, RTOSLab. SNU.
 * 
 * Description: 
 ********************************************************/
#include <core/eos.h>

static eos_counter_t system_timer;

int8u_t eos_init_counter(eos_counter_t *counter, int32u_t init_value) {
	counter->tick = init_value;
	counter->alarm_queue = NULL;
	return 0;
}

void eos_set_alarm(eos_counter_t* counter, eos_alarm_t* alarm, int32u_t timeout, void (*entry)(void *arg), void *arg) {
	if(timeout == 0 || entry == NULL) return;

	// make alarm
	alarm->timeout = counter->tick + timeout;
	alarm->handler = entry;
	alarm->arg = arg;

	// push on alarm queue
	_os_node_t *new_node = malloc(sizeof(_os_node_t));
	new_node->ptr_data = (void*)alarm;
	new_node->priority = counter->tick + timeout;
	//PRINT("%p, new alarm period: %d, timeout: %d \n", new_node, alarm->timeout, new_node->priority);

	_os_add_node_priority(&(counter->alarm_queue), new_node);
}

eos_counter_t* eos_get_system_timer() {
	return &system_timer;
}

void eos_trigger_counter(eos_counter_t* counter) {

	// update tick
	counter->tick++;
	PRINT("--------------------tick: %d---------------------\n", counter->tick);
	// push interrupted task in to ready queue(because it does not have to wait)
	_os_wakeup_sleeping_task((void*)eos_get_current_task());

	// search for all finished alarms and push on ready queue
	_os_node_t *node = counter->alarm_queue;
	while((counter->alarm_queue != NULL) && (counter->tick == node->priority)) {
		void (*handler)(void *arg) = ((eos_alarm_t*)(node->ptr_data))->handler;
		void *arg = ((eos_alarm_t*)(node->ptr_data))->arg;
		handler(arg);

		_os_node_t *new_node = node->next;
		_os_remove_node(&(counter->alarm_queue), node);
		free(node);
		node = new_node;
	}

	eos_schedule();
}

/* Timer interrupt handler */
static void timer_interrupt_handler(int8s_t irqnum, void *arg) {
	/* trigger alarms */
	eos_trigger_counter(&system_timer);
}

void _os_init_timer() {
	eos_init_counter(&system_timer, 0);

	/* register timer interrupt handler */
	eos_set_interrupt_handler(IRQ_INTERVAL_TIMER0, timer_interrupt_handler, NULL);
}
