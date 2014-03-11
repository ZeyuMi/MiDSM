#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/syn.h"
#include "minunit.h"
int tests_run = 0;

int hostnum;
int myhostid;

static char *test_initsyn(){
	extern milock_t locks[LOCK_NUM];
	extern int waitFlag;
	extern int myLocks[LOCK_NUM];
	extern int barrierFlags[MAX_HOST_NUM];
	extern int lasthostid;

	initsyn();
	mu_assert("syn1", locks[0].state == FREE);
	mu_assert("syn1.1", locks[0].owner == -1);
	mu_assert("syn2", locks[0].lasthostid == -1);
	int i;
	for(i = 0; i < MAX_HOST_NUM; i++){
		mu_assert("syn3", locks[0].waitingList[i] == -1);
	}
	mu_assert("syn3.1", locks[0].waitingListCount == 0);
	mu_assert("syn4", waitFlag == 0);
	mu_assert("syn5", myLocks[0] == 0);
	mu_assert("syn6", myLocks[200] == 0);
	mu_assert("syn7", barrierFlags[2] == 0);
	mu_assert("syn8", barrierFlags[0] == 0);
	mu_assert("syn8.1", lasthostid == -1);
	
	return 0;
}


static char *test_graspLock(){
	extern milock_t locks[LOCK_NUM];

	initsyn();

	hostnum = 4;
	myhostid = 0;
			
	mu_assert("syn9", graspLock(0, 2) == -1);
	mu_assert("syn10", locks[0].state == LOCKED);
	mu_assert("syn10.1", locks[0].owner == 2);
	mu_assert("syn10.2", locks[0].waitingListCount == 0);
	mu_assert("syn10.3", locks[0].waitingList[0] == -1);
	mu_assert("syn10.4", graspLock(0, 2) == -4);

	mu_assert("syn11", graspLock(-1, 2) == -5);
	mu_assert("syn12", graspLock(2000, 2) == -5);
	mu_assert("syn13", graspLock(2, 5) == -5);
	mu_assert("syn14", graspLock(2, -1) == -5);
	mu_assert("syn15", graspLock(-1, -1) == -5);

	mu_assert("syn16", graspLock(1, 2) == -2);
	mu_assert("syn17", graspLock(6, 2) == -2);
	mu_assert("syn18", graspLock(11, 2) == -2);

	mu_assert("syn19", graspLock(0, 3) == -3);
	mu_assert("syn20", locks[0].waitingList[0] == 3);
	mu_assert("syn20.1", locks[0].waitingListCount == 1);
	mu_assert("syn21", locks[0].waitingList[1] == -1);

	mu_assert("syn22", graspLock(0, 1) == -3);
	mu_assert("syn23", locks[0].waitingList[0] == 3);
	mu_assert("syn23.1", locks[0].waitingList[1] == 1);
	mu_assert("syn23.2", locks[0].waitingList[2] == -1);
	mu_assert("syn24", locks[0].waitingListCount == 2);

	locks[12].lasthostid = 2;
	mu_assert("syn24.1", graspLock(12, 1) == 2);
	mu_assert("syn24.2", locks[12].state == LOCKED);
	mu_assert("syn24.3", locks[12].owner == 1);
	mu_assert("syn24.5", locks[12].waitingListCount == 0);
	mu_assert("syn24.6", locks[12].waitingList[0] == -1);

	return 0;
}


static char *test_freeLock(){
	extern milock_t locks[LOCK_NUM];

	initsyn();

	hostnum = 4;
	myhostid = 0;
			
	mu_assert("syn25", graspLock(0, 2) == -1);
	mu_assert("syn26", locks[0].state == LOCKED);
	mu_assert("syn26.1", locks[0].owner == 2);
	mu_assert("syn26.2", locks[0].waitingList[0] == -1);
	mu_assert("syn26.3", locks[0].waitingListCount == 0);
	mu_assert("syn27", graspLock(0, 3) == -3);
	mu_assert("syn28", locks[0].waitingList[0] == 3);
	mu_assert("syn29", locks[0].waitingListCount == 1);


	mu_assert("syn30", graspLock(0, 1) == -3);

	mu_assert("syn31", locks[0].waitingList[0] == 3);
	mu_assert("syn31.1", locks[0].waitingList[1] == 1);
	mu_assert("syn32", locks[0].waitingListCount == 2);

	mu_assert("syn33", freeLock(0, 2) == 3);
	mu_assert("syn34", locks[0].state == LOCKED);
	mu_assert("syn34.1", locks[0].owner == 3);
	mu_assert("syn34.2", locks[0].lasthostid == 2);
	mu_assert("syn35", locks[0].waitingList[0] == 1);
	mu_assert("syn36", locks[0].waitingListCount == 1);
	mu_assert("syn37", freeLock(0, 3) == 1);
	mu_assert("syn38", locks[0].state == LOCKED);
	mu_assert("syn38.1", locks[0].owner == 1);
	mu_assert("syn38.2", locks[0].lasthostid == 3);
	mu_assert("syn39", locks[0].waitingList[0] == -1);
	mu_assert("syn39.1", locks[0].waitingListCount == 0);
	mu_assert("syn40", freeLock(0, 1) == -5);
	mu_assert("syn41", locks[0].state == FREE);
	mu_assert("syn41.1", locks[0].owner == -1);
	mu_assert("syn41.2", locks[0].lasthostid == 1);
	mu_assert("syn41.3", locks[0].waitingList[0] == -1);
	mu_assert("syn41.4", locks[0].waitingListCount == 0);

	mu_assert("syn42", freeLock(-1, 2) == -1);
	mu_assert("syn43", freeLock(1024, 2) == -1);
	mu_assert("syn43.1", freeLock(1, -1) == -1);
	mu_assert("syn43.2", freeLock(1, 4) == -1);

	mu_assert("syn44", freeLock(2, 1) == -2);


	mu_assert("syn47", graspLock(4, 3) == -1);
	mu_assert("syn48", locks[4].state == LOCKED);
	mu_assert("syn48.1", locks[4].owner == 3);
	mu_assert("syn49", freeLock(4, 3) == -5);
	mu_assert("syn49.1", locks[4].lasthostid == 3);
	mu_assert("syn49.2", locks[4].owner == -1);
	mu_assert("syn50", locks[4].state == FREE);

	mu_assert("syn50.1", graspLock(4, 3) == 3);
	mu_assert("syn50.2", locks[4].state == LOCKED);
	mu_assert("syn50.3", locks[4].owner == 3);
	mu_assert("syn50.4", freeLock(4, 2) == -4);
	mu_assert("syn50.5", locks[4].state == LOCKED);
	mu_assert("syn50.6", locks[4].owner == 3);
	mu_assert("syn50.7", freeLock(4, 3) == -5);
	mu_assert("syn50.8", locks[4].state == FREE);
	mu_assert("syn50.9", locks[4].owner == -1);
	mu_assert("syn50.10", locks[4].lasthostid == 3);

	return 0;
}


static char *test_checkBarrierFlags(){
	extern mimsg_t msg;
	extern parametertype;
	extern sendMsgCalled;
	extern int barrierFlags[MAX_HOST_NUM];
	hostnum = 2;
	myhostid = 0;
	initsyn();
	
	int i;
	for(i = 0; i < hostnum; i++){
		barrierFlags[i] = 1;
	}
	
	mu_assert("syn51", checkBarrierFlags() == 0);
	mu_assert("syn52", msg.from == 0);
	mu_assert("syn53", msg.to == 1);
	mu_assert("syn54", msg.command == EXIT_BARRIER);
	mu_assert("syn55", sendMsgCalled == 1);
	mu_assert("syn56", parametertype == 0);
	sendMsgCalled = 0;
	barrierFlags[1] = 0;
	mu_assert("syn57", checkBarrierFlags() == -1);
	mu_assert("syn58", sendMsgCalled == 0);

	myhostid = 1;
	mu_assert("syn59", checkBarrierFlags() == -2);

	return 0;
}


static char *test_grantLock(){
	extern mimsg_t msg;
	extern parametertype;
	extern sendMsgCalled;
	extern nextFreeMsgInQueueCalled;
	extern milock_t locks[LOCK_NUM];

	initsyn();
	sendMsgCalled = 0;
	nextFreeMsgInQueueCalled = 0;
	
	hostnum = 4;
	myhostid = 0;

	grantLock(-1, 3);	
	mu_assert("syn60", sendMsgCalled == 0);
	mu_assert("syn61", nextFreeMsgInQueueCalled == 0);

	grantLock(1024, 3);	
	mu_assert("syn62", sendMsgCalled == 0);
	mu_assert("syn63", nextFreeMsgInQueueCalled == 0);

	grantLock(0, -1);	
	mu_assert("syn64", sendMsgCalled == 0);
	mu_assert("syn65", nextFreeMsgInQueueCalled == 0);

	grantLock(0, 4);	
	mu_assert("syn66", sendMsgCalled == 0);
	mu_assert("syn67", nextFreeMsgInQueueCalled == 0);

	grantLock(1, 3);	
	mu_assert("syn68", sendMsgCalled == 0);
	mu_assert("syn69", nextFreeMsgInQueueCalled == 0);

	grantLock(0, 0);	
	mu_assert("syn70", sendMsgCalled == 0);
	mu_assert("syn71", nextFreeMsgInQueueCalled == 0);

	grantLock(8, 1);	
	mu_assert("syn72", sendMsgCalled == 1);
	mu_assert("syn73", nextFreeMsgInQueueCalled == 1);
	mu_assert("syn74", parametertype == 0);
	mu_assert("syn75", msg.from == 0);
	mu_assert("syn76", msg.to == 1);
	mu_assert("syn77", msg.command == GRANT_LOCK);
	mu_assert("syn78", strtol(msg.data, NULL, 10) == 8);
	mu_assert("syn79", *((int *)(msg.data + sizeof(int))) == -1);

	memset(&msg, 0, sizeof(mimsg_t));
	locks[8].lasthostid = 2;
	grantLock(8, 1);	
	mu_assert("syn80", sendMsgCalled == 1);
	mu_assert("syn81", nextFreeMsgInQueueCalled == 1);
	mu_assert("syn82", parametertype == 0);
	mu_assert("syn83", msg.from == 0);
	mu_assert("syn84", msg.to == 1);
	mu_assert("syn85", msg.command == GRANT_LOCK);
	mu_assert("syn86", strtol(msg.data, NULL, 10) == 8);
	mu_assert("syn87", *((int *)(msg.data + sizeof(int))) == 2);

	return 0;
}


static char *all_tests(){
	mu_run_test(test_initsyn);
	mu_run_test(test_graspLock);
	mu_run_test(test_freeLock);
	mu_run_test(test_checkBarrierFlags);
	mu_run_test(test_grantLock);
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

