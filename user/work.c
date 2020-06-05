#include <core/eos.h>

static eos_tcb_t tcb1;
static eos_tcb_t tcb2;
static eos_tcb_t tcb3;
static int8u_t stack1[8096];
static int8u_t stack2[8096];
static int8u_t stack3[8096];
static int8u_t queue1[10];
static int8u_t queue2[10];
eos_mqueue_t mq1;
eos_mqueue_t mq2;

void print_status(eos_mqueue_t *m1, eos_mqueue_t *m2) {
	int32u_t m1_count = (m1->getsem).count;
	int32u_t m2_count = (m2->getsem).count;

	int8u_t *q1 = malloc(sizeof(int8u_t)*5);
	int8u_t *q2 = malloc(sizeof(int8u_t)*5);

	for(int i = 0; i < m1_count; i++) 
		q1[i] = '*';	

	for(int i = 0; i < m2_count; i++)
		q2[i] = '*';	

	PRINT("mq1 |%s\n", q1);
	PRINT("mq2 |%s\n", q2);
}

static void sender_task(void *arg) {
	int8u_t *data = "xy";

	while (1) {
		eos_send_message(&mq1, data, 0) ;
		print_status(&mq1, &mq2);
		eos_send_message(&mq2, data, 0) ;
		print_status(&mq1, &mq2);
		eos_sleep(0);
	}
}

static void receiver_task1(void *arg) {
	int8u_t data[2];

	while (1) {
		eos_receive_message(&mq1, data, 0);
		PRINT("received message: %s\n", data);
		print_status(&mq1, &mq2);
		eos_sleep(0);
	}
}

static void receiver_task2(void *arg) {
	int8u_t data[2];

	while (1) {
		eos_receive_message(&mq2, data, 0);
		PRINT("received message: %s\n", data);
		print_status(&mq1, &mq2);
		eos_sleep(0);
	}
}

void eos_user_main() {
	eos_create_task(&tcb1, (addr_t)stack1, 8096, sender_task, NULL, 50);
	eos_create_task(&tcb2, (addr_t)stack2, 8096, receiver_task1, NULL, 10);
	eos_create_task(&tcb3, (addr_t)stack3, 8096, receiver_task2, NULL, 20);

	eos_set_period(&tcb1, 2);
	eos_set_period(&tcb2, 4);
	eos_set_period(&tcb3, 6);

	eos_init_mqueue(&mq1, queue1, 5, 2, 1);
	eos_init_mqueue(&mq2, queue2, 5, 2, 1);
}
