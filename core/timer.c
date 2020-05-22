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

	alarm->timeout = timeout;
	alarm->handler = entry;
	alarm->arg = arg;

	_os_node_t *new_node = malloc(sizeof(struct _os_node_t));
	new_node->ptr_data = (void *) alarm;
	new_node->priority = timeout;
	_os_add_node_priority(&(counter->alarm_queue), new_node);	
}

eos_counter_t* eos_get_system_timer() {
	return &system_timer;
}

void eos_trigger_counter(eos_counter_t* counter) {
	PRINT("tick\n");
	counter->tick++;
	if(counter->tick == counter->alarm_queue->timeout) {
		_os_node_t *node = counter->alarm_queue;
		eos_alarm_t *alarm = (eos_alarm_t *)(node->ptr_data);
		void (*handler)(void *arg) = alarm->handler;
		void *arg = alarm->arg;
		_os_remove_node(&(counter->alarm_queue), counter->alarm_queue);
		handler(arg);
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
