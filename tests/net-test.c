#include <stdio.h>
#include "../src/net.h"
#include "minunit.h"

int tests_run = 0;

static char *test_newMsg(){
	mimsg_t *msg = newMsg();
	mu_assert("net1", msg != NULL);
	mu_assert("net2", msg->from == -1);
	mu_assert("net3", msg->to == -1);
	mu_assert("net4", msg->command == -1);
	mu_assert("net4.1", msg->size == 0);
	mu_assert("net4.2", msg->seqno == 0);
	return 0;
}

static char *test_freeMsg(){
	int r = freeMsg(NULL);
	mu_assert("net5", r == -1);
	mimsg_t *msg = newMsg();
	r = freeMsg(msg);
	mu_assert("net6", r == 0);
	return 0;
}

static char *test_msgEnqueue(){
	mimsg_t *msg = newMsg();
	mu_assert("net7", msgEnqueue(0, msg) == 0);
	mimsg_t *m = queueTop(0);
	mu_assert("net8", m->from == -1);
	mu_assert("net9", msgEnqueue(1, msg) == 0);
	m = queueTop(1);
	mu_assert("net10", m->from == -1);
	mu_assert("net11", msgEnqueue(2, msg) == -1);
	mu_assert("net12", msgEnqueue(0, NULL) == -1);
	int i = 0;
	for(; i < 19; i++)
		msgEnqueue(0, msg);
	mu_assert("net13", msgEnqueue(0, msg) == -2);
	for(i = 0; i < 19; i++)
		msgEnqueue(1, msg);
	mu_assert("net14", msgEnqueue(1, msg) == -2);

	for(i = 0; i < 20; i++)
		msgDequeue(0);
	for(i = 0; i < 20; i++)
		msgDequeue(1);
	return 0;
}

static char *test_queueTop(){
	mu_assert("net15", queueTop(0) == NULL);
	mu_assert("net16", queueTop(1) == NULL);
	mu_assert("net17", queueTop(2) == NULL);
	mimsg_t *msg = newMsg();
	mu_assert("net18", msgEnqueue(0, msg) == 0);
	mimsg_t *m = queueTop(0);
	mu_assert("net19", m->from == -1);
	mu_assert("net20", msgEnqueue(1, msg) == 0);
	m = queueTop(1);
	mu_assert("net21", m->from == -1);
	int i = 0;
	for(i = 0; i < 20; i++)
		msgDequeue(0);
	for(i = 0; i < 20; i++)
		msgDequeue(1);
	return 0;
}

static char *test_msgDequeue(){
	mu_assert("net22", msgDequeue(0) == -2);
	mu_assert("net23", msgDequeue(1) == -2);
	mu_assert("net24", msgDequeue(2) == -1);
	mimsg_t *msg = newMsg();
	mu_assert("net25", msgEnqueue(0, msg) == 0);
	mimsg_t *m = queueTop(0);
	mu_assert("net26", m->from == -1);
	mu_assert("net27", msgDequeue(0) == 0);
	mu_assert("net28", queueTop(0) == NULL);

	mu_assert("net29", msgEnqueue(1, msg) == 0);
	m = queueTop(1);
	mu_assert("net30", m->from == -1);
	mu_assert("net31", msgDequeue(1) == 0);
	mu_assert("net32", queueTop(1) == NULL);
	int i = 0;
	for(i = 0; i < 20; i++)
		msgDequeue(0);
	for(i = 0; i < 20; i++)
		msgDequeue(1);
	return 0;
}


static char *test_apendMsgData(){
	mimsg_t *msg = newMsg();
	
	mu_assert("net33", (msg->data)[0] == 0);
	char a = 3;
	mu_assert("net34", apendMsgData(msg, &a, 1) == 0);
	mu_assert("net35", (msg->data)[0] == 3);
	mu_assert("net36", (msg->data)[1] == 0);
	mu_assert("net37", msg->size == 1);

	mu_assert("net38", apendMsgData(NULL, &a, 1) == -1);
	mu_assert("net39", apendMsgData(msg, NULL, 1) == -1);
	mu_assert("net39", apendMsgData(msg, &a, 0) == -1);
	mu_assert("net40", apendMsgData(msg, &a, MAX_MSG_SIZE - MSG_HEAD_SIZE +1) == -2);
	freeMsg(msg);
}

static char *all_tests(){
	mu_run_test(test_newMsg);
	mu_run_test(test_freeMsg);
	mu_run_test(test_msgEnqueue);
	mu_run_test(test_queueTop);
	mu_run_test(test_msgDequeue);
	mu_run_test(test_apendMsgData);
	return 0;
}


int main(){
	char *result = all_tests();
	if(result != 0){
		printf("%s\n", result);
	}else{
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run : %d\n", tests_run);
	return result != 0;
}
