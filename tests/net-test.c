#include <stdio.h>
#include "../src/net.h"
#include "minunit.h"

int tests_run = 0;

host_t hosts[2];
int hostnum = 2;
int myhostid = 3;

static char *test_msgEnqueue(){
	mimsg_t *msg = nextFreeMsgInQueue(0);
	mu_assert("net7", msgEnqueue(0) == 0);
	mimsg_t *m = queueTop(0);
	mu_assert("net8", m->from == -1);
	mu_assert("net9", msgEnqueue(1) == 0);
	mu_assert("net11", msgEnqueue(2) == -1);
	mu_assert("net12", msgEnqueue(-1) == -1);
	int i = 0;
	for(; i < 20; i++)
		msgEnqueue(0);
	mu_assert("net13", msgEnqueue(0) == -2);
	for(i = 0; i < 20; i++)
		msgEnqueue(1);
	mu_assert("net14", msgEnqueue(1) == -2);

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
	mimsg_t *msg = nextFreeMsgInQueue(0);
	mu_assert("net18", msgEnqueue(0) == 0);
	mimsg_t *m = nextFreeMsgInQueue(1);
	mu_assert("net19", m->from == -1);
	mu_assert("net20", msgEnqueue(1) == 0);
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
	mimsg_t *msg = nextFreeMsgInQueue(0);
	mu_assert("net25", msgEnqueue(0) == 0);
	mimsg_t *m = queueTop(0);
	mu_assert("net26", m->from == -1);
	mu_assert("net27", msgDequeue(0) == 0);
	mu_assert("net28", queueTop(0) == NULL);

	msg = nextFreeMsgInQueue(1);
	mu_assert("net29", msgEnqueue(1) == 0);
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
	mimsg_t *msg = nextFreeMsgInQueue(0);
	
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
	return 0;
}


static char *test_nextFreeMsgInQueue(){
	mimsg_t *msg = nextFreeMsgInQueue(0);
	
	mu_assert("net41", msg != NULL);
	mu_assert("net42", msg->from == -1);
	mu_assert("net43", msg->to == -1);
	mu_assert("net44", msg->command == -1);
	mu_assert("net45", msg->size == 0);
	mu_assert("net46", msg->seqno == -1);
	mu_assert("net47", (msg->data)[0] == 0);
	msg->size = 12345;
	mu_assert("net48", msgEnqueue(0) == 0);
	msg = queueTop(0);
	mu_assert("net49", msg != NULL);
	mu_assert("net50", msg->size == 12345);
	mu_assert("net50.1", msg->timestamp[0] == -1);
	mu_assert("net50.2", msg->timestamp[1] == -1);
	mu_assert("net50.3", msg->timestamp[2] == -1);
	mu_assert("net50.4", msg->timestamp[MAX_HOST_NUM-1] == -1);

	msg = nextFreeMsgInQueue(1);
	
	mu_assert("net51", msg != NULL);
	mu_assert("net52", msg->from == -1);
	mu_assert("net53", msg->to == -1);
	mu_assert("net54", msg->command == -1);
	mu_assert("net55", msg->size == 0);
	mu_assert("net56", msg->seqno == -1);
	mu_assert("net57", (msg->data)[0] == 0);
	msg->size = 12345;
	mu_assert("net58", msgEnqueue(1) == 0);
	msg = queueTop(1);
	mu_assert("net59", msg != NULL);
	mu_assert("net60", msg->size == 12345);
	mu_assert("net60.1", msg->timestamp[0] == -1);
	mu_assert("net60.2", msg->timestamp[1] == -1);
	mu_assert("net60.3", msg->timestamp[2] == -1);
	mu_assert("net60.4", msg->timestamp[MAX_HOST_NUM-1] == -1);


	int i = 0;
	for(i = 0; i < 20; i++){
		msgEnqueue(0);
	}
	mu_assert("net61", nextFreeMsgInQueue(0) == NULL);
	mu_assert("net62", nextFreeMsgInQueue(-1) == NULL);
	mu_assert("net63", nextFreeMsgInQueue(3) == NULL);
	for(i = 0; i < 20; i++){
		msgDequeue(0);
	}
	for(i = 0; i < 20; i++){
		msgEnqueue(1);
	}
	mu_assert("net64", nextFreeMsgInQueue(1) == NULL);
	for(i = 0; i < 20; i++){
		msgDequeue(1);
	}
	return 0;
}

static char *all_tests(){
	mu_run_test(test_msgEnqueue);
	mu_run_test(test_queueTop);
	mu_run_test(test_msgDequeue);
	mu_run_test(test_apendMsgData);
	mu_run_test(test_nextFreeMsgInQueue);
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
