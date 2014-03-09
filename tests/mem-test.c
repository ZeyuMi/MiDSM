#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/mem.h"
#include "../src/init.h"
#include "minunit.h"
int tests_run = 0;

int myhostid;
int hostnum;
extern page_t pageArray[MAX_PAGE_NUM];

static char *test_isAfterInterval(){
	int a[MAX_HOST_NUM];
	int b[MAX_HOST_NUM];
	memset(a, 0, MAX_HOST_NUM);
	memset(b, 0, MAX_HOST_NUM);

	hostnum = 4;
	
	a[0] = 1;
	b[0] = 2;
	mu_assert("mem1", isAfterInterval(a, b) == 0);

	a[0] = 1;
	b[0] = 1;
	mu_assert("mem2", isAfterInterval(a, b) == 0);

	a[1] = 2;
	b[1] = 1;
	mu_assert("mem3", isAfterInterval(a, b) == 1);

	a[2] = 2;
	b[2] = 0;
	mu_assert("mem4", isAfterInterval(a, b) == 1);

	a[4] = 2;
	b[4] = 3;
	mu_assert("mem5", isAfterInterval(a, b) == 1);


	mu_assert("mem6", isAfterInterval(NULL, b) == -1);
	mu_assert("mem7", isAfterInterval(a, NULL) == -1);
	mu_assert("mem8", isAfterInterval(NULL, NULL) == -1);
	
	return 0;
}


static char *test_createTwinPage(){
	memset(pageArray, 0, MAX_PAGE_NUM * sizeof(page_t));
	pageArray[0].address = malloc(PAGESIZE);
	pageArray[0].state = RDONLY;		
	pageArray[0].twinPage = NULL;
	memset(pageArray[0].address, 0, PAGESIZE);
	((int *)(pageArray[0].address))[2] = 3;
	((int *)(pageArray[0].address))[1023] = 1023;
	mu_assert("mem9", createTwinPage(0) == 0);
	mu_assert("mem10", ((int *)pageArray[0].twinPage)[2] == 3);
	mu_assert("mem11", ((int *)pageArray[0].twinPage)[1023] == 1023);
	mu_assert("mem12", createTwinPage(0) == -1);
	mu_assert("mem13", createTwinPage(1) == -1);
	mu_assert("mem14", createTwinPage(-1) == -1);
	mu_assert("mem15", createTwinPage(MAX_PAGE_NUM) == -1);
	pageArray[1].address = malloc(PAGESIZE);
	pageArray[1].state = MISS;		
	pageArray[1].twinPage = NULL;
	mu_assert("mem16", createTwinPage(1) == -1);
	free(pageArray[0].address);
	free(pageArray[0].twinPage);
	free(pageArray[1].address);

	return 0;
}


static char *test_freeTwinPage(){
	memset(pageArray, 0, MAX_PAGE_NUM * sizeof(page_t));
	pageArray[0].address = malloc(PAGESIZE);
	pageArray[0].state = RDONLY;		
	pageArray[0].twinPage = NULL;
	memset(pageArray[0].address, 0, PAGESIZE);
	((int *)(pageArray[0].address))[2] = 3;
	((int *)(pageArray[0].address))[1023] = 1023;
	mu_assert("mem17", createTwinPage(0) == 0);
	mu_assert("mem18", ((int *)pageArray[0].twinPage)[2] == 3);
	mu_assert("mem19", ((int *)pageArray[0].twinPage)[1023] == 1023);
	mu_assert("mem20", freeTwinPage(0) == 0);
	mu_assert("mem21", pageArray[0].twinPage == NULL);
	
	mu_assert("mem22", freeTwinPage(-1) == -1);
	mu_assert("mem23", freeTwinPage(MAX_PAGE_NUM) == -1);
	mu_assert("mem24", freeTwinPage(0) == -1);
	mu_assert("mem25", freeTwinPage(3) == -1);

	free(pageArray[0].address);
	return 0;
}


static char *test_applyDiff(){
	void *memory1 = malloc(PAGESIZE);
	void *memory2 = malloc(PAGESIZE);
	memset(memory1, 0, PAGESIZE);
	memset(memory2, 0, PAGESIZE);
	((char *)memory1)[3] = 30;
	((char *)memory1)[PAGESIZE -1] = 11;
	((char *)memory2)[3] = -11;
	((char *)memory2)[PAGESIZE -1] = 5;
	mu_assert("mem26", applyDiff(memory1, memory2) == 0);
	mu_assert("mem27", ((char *)memory1)[0] == 0);
	mu_assert("mem28", ((char *)memory1)[3] == 19);
	mu_assert("mem29", ((char *)memory1)[PAGESIZE - 1] == 16);

	mu_assert("mem30", applyDiff(memory1, NULL) == -1);
	mu_assert("mem31", applyDiff(NULL, memory2) == -1);
	mu_assert("mem32", applyDiff(NULL, NULL) == -1);

	free(memory1);
	free(memory2);
	return 0;
}


static char *test_createLocalDiff(){
	void *memory1 = malloc(PAGESIZE);
	void *memory2 = malloc(PAGESIZE);
	memset(memory1, 0, PAGESIZE);
	memset(memory2, 0, PAGESIZE);
	((char *)memory1)[3] = 30;
	((char *)memory1)[PAGESIZE -1] = 11;
	((char *)memory2)[3] = -11;
	((char *)memory2)[PAGESIZE -1] = 5;
	void *memory3 = createLocalDiff(memory1, memory2);
	mu_assert("mem33", memory3 != NULL);
	mu_assert("mem34", ((char *)memory3)[0] == 0);
	mu_assert("mem35", ((char *)memory3)[3] == 41);
	mu_assert("mem36", ((char *)memory3)[PAGESIZE - 1] == 6);

	mu_assert("mem37", createLocalDiff(memory1, NULL) == NULL);
	mu_assert("mem38", createLocalDiff(NULL, memory2) == NULL);
	mu_assert("mem39", createLocalDiff(NULL, NULL) == NULL);

	free(memory1);
	free(memory2);
	free(memory3);
	return 0;
}


static char *test_createWriteNotice(){
	extern interval_t *intervalNow;

	interval_t tempInterval;
	intervalNow = &tempInterval;
	myhostid = 0;
	
	memset(intervalNow, 0, sizeof(interval_t));
	intervalNow->notices = NULL;
	intervalNow->next = NULL;
	intervalNow->isBarrier = 0;

	memset(pageArray, 0, MAX_PAGE_NUM * sizeof(page_t));
	pageArray[0].address = malloc(PAGESIZE);
	pageArray[0].state = RDONLY;	
	
	mu_assert("mem40", createWriteNotice(0) == 0);
	writenotice_t *wn = pageArray[0].notices[myhostid];
	mu_assert("mem41",  wn != NULL);
	mu_assert("mem42",  wn->interval == intervalNow);
	mu_assert("mem43",  wn->nextInPage == NULL);
	mu_assert("mem44",  wn->nextInInterval == NULL);
	mu_assert("mem45",  wn->diffAddress == NULL);
	mu_assert("mem45.1", wn->pageIndex == 0);
	mu_assert("mem46",  intervalNow->notices == wn);

	mu_assert("mem47", createWriteNotice(0) == 0);
	writenotice_t *wn2 = pageArray[0].notices[myhostid];
	mu_assert("mem48",  wn2 != NULL);
	mu_assert("mem49",  wn2->interval == intervalNow);
	mu_assert("mem50",  wn2->nextInPage == wn);
	mu_assert("mem51",  wn2->nextInInterval == NULL);
	mu_assert("mem52",  wn2->diffAddress == NULL);
	mu_assert("mem52.1",  wn2->pageIndex == 0);
	mu_assert("mem53",  intervalNow->notices == wn);
	mu_assert("mem54",  intervalNow->notices->nextInInterval == wn2);

	mu_assert("mem55", createWriteNotice(-1) == -1);
	mu_assert("mem56", createWriteNotice(MAX_PAGE_NUM) == -1);
	pageArray[1].address = malloc(PAGESIZE);
	pageArray[1].state = WRITE;	
	mu_assert("mem57", createWriteNotice(1) == -1);
	pageArray[1].state = UNMAP;	
	mu_assert("mem58", createWriteNotice(1) == -1);
	pageArray[1].state = MISS;	
	mu_assert("mem59", createWriteNotice(1) == -1);
	pageArray[1].state = INVALID;	
	mu_assert("mem60", createWriteNotice(1) == -1);

	return 0;
}


static char *test_addNewInterval(){
	extern interval_t *intervalNow;
	extern proc_t procArray[MAX_HOST_NUM];

	interval_t tempInterval;
	intervalNow = &tempInterval;
	myhostid = 0;
	hostnum = 4;
	
	memset(intervalNow, 0, sizeof(interval_t));
	intervalNow->notices = NULL;
	intervalNow->next = NULL;
	intervalNow->isBarrier = 0;

	memset(procArray, 0, MAX_HOST_NUM * sizeof(proc_t));
	procArray[0].hostid = 0;
	procArray[0].intervalList = intervalNow;	
	addNewInterval();
	mu_assert("mem61", (intervalNow->timestamp)[0] == 1);
	mu_assert("mem62", (intervalNow->timestamp)[1] == 0);
	mu_assert("mem63", (intervalNow->timestamp)[2] == 0);
	mu_assert("mem64", (intervalNow->timestamp)[3] == 0);
	mu_assert("mem65", procArray[0].intervalList == intervalNow);
	mu_assert("mem66", procArray[0].intervalList->next == &tempInterval);

	return 0;
}


static char *test_incorporateWnPacket(){
	extern proc_t procArray[MAX_HOST_NUM];
	extern page_t pageArray[MAX_PAGE_NUM];
	extern interval_t *intervalNow;

	interval_t tempInterval;
	intervalNow = &tempInterval;
	myhostid = 0;
	hostnum = 4;
	
	memset(intervalNow, 0, sizeof(interval_t));
	intervalNow->notices = NULL;
	intervalNow->next = NULL;
	intervalNow->isBarrier = 0;

	memset(procArray, 0, MAX_HOST_NUM * sizeof(proc_t));
	procArray[0].hostid = 0;
	procArray[0].intervalList = intervalNow;	
	
	mu_assert("mem67", incorporateWnPacket(NULL) == -1);
	
	wnPacket_t *packet = malloc(sizeof(wnPacket_t));

	memset(packet, 0, sizeof(wnPacket_t));
	packet->hostid = 1;
	(packet->timestamp)[1] = 1;
	packet->wnCount = 2;
	(packet->wnArray)[0] = 2;
	(packet->wnArray)[1] = 1023;
	
	mu_assert("mem68", incorporateWnPacket(packet) == 0);
	mu_assert("mem69", (procArray[1].intervalList->timestamp)[1] == 1);
	mu_assert("mem70", (procArray[1].intervalList->timestamp)[0] == 0);
	mu_assert("mem71", (procArray[1].intervalList->timestamp)[2] == 0);
	mu_assert("mem72", (procArray[1].intervalList->timestamp)[3] == 0);
	mu_assert("mem73", (procArray[1].intervalList)->notices == pageArray[2].notices[1]);
	mu_assert("mem74", (procArray[1].intervalList)->notices->nextInInterval == pageArray[1023].notices[1]);
	mu_assert("mem75", (procArray[1].intervalList)->notices->nextInInterval->nextInInterval == NULL);
	mu_assert("mem76", (procArray[1].intervalList)->notices->nextInPage == NULL);
	mu_assert("mem77", (procArray[1].intervalList)->notices->nextInInterval->nextInPage == NULL);
	mu_assert("mem78", (procArray[1].intervalList)->notices->interval == procArray[1].intervalList);
	mu_assert("mem79", (procArray[1].intervalList)->notices->nextInInterval->interval == procArray[1].intervalList);
	mu_assert("mem80", (procArray[1].intervalList)->notices->diffAddress == NULL);
	mu_assert("mem81", (procArray[1].intervalList)->notices->nextInInterval->diffAddress == NULL);
	mu_assert("mem81.1", (procArray[1].intervalList)->notices->nextInInterval->pageIndex == 1023);
	mu_assert("mem81.2", (procArray[1].intervalList)->notices->pageIndex == 2);

	memset(packet, 0, sizeof(wnPacket_t));
	packet->hostid = 1;
	(packet->timestamp)[1] = 2;
	packet->wnCount = 1;
	(packet->wnArray)[0] = 2;
	
	mu_assert("mem82", incorporateWnPacket(packet) == 0);
	mu_assert("mem83", (procArray[1].intervalList->timestamp)[1] == 2);
	mu_assert("mem84", (procArray[1].intervalList->timestamp)[0] == 0);
	mu_assert("mem85", (procArray[1].intervalList->timestamp)[2] == 0);
	mu_assert("mem86", (procArray[1].intervalList->timestamp)[3] == 0);
	mu_assert("mem87", (procArray[1].intervalList->next->timestamp)[0] == 0);
	mu_assert("mem88", (procArray[1].intervalList->next->timestamp)[1] == 1);
	mu_assert("mem89", (procArray[1].intervalList->next->timestamp)[2] == 0);
	mu_assert("mem90", (procArray[1].intervalList->next->timestamp)[3] == 0);


	mu_assert("mem91", (procArray[1].intervalList)->notices == pageArray[2].notices[1]);
	mu_assert("mem92", (procArray[1].intervalList)->notices->nextInInterval == NULL);
	mu_assert("mem93", (procArray[1].intervalList)->notices->nextInPage->interval == procArray[1].intervalList->next);
	mu_assert("mem94", (procArray[1].intervalList)->notices->interval == procArray[1].intervalList);
	mu_assert("mem95", (procArray[1].intervalList)->notices->diffAddress == NULL);
	mu_assert("mem96", (procArray[1].intervalList)->notices->pageIndex == 2);

	return 0;
}


static char *test_addWNIIntoPacketForHost(){
	wnPacket_t *packet = malloc(sizeof(wnPacket_t));

	memset(packet, 0, sizeof(wnPacket_t));
	hostid = 1;
	int timestamp[MAX_HOST_NUM];
	memset(timestamp, 0, MAX_HOST_NUM * sizeof(int)):
	timestamp[0] = 1;
	timestamp[3] = 3;
	
	int wnCount = 3;
	int i;
	writenotice_t *notices = NULL;
	writenotice_t *lastwn = NULL;
	writenotice_t *wn = NULL;
	for(i = 0; i < wnCount; i++){
		wn = malloc(sizeof(writenotice_t));
		wn->nextInInterval = NULL;
		wn->pageIndex = i;
		if(lastwn == NULL){
			lastwn = notices = wn;
		}else{
			lastwn->nextInInterval = wn;
		}
		lastwn = wn;
	}
	
	//parameter error check
	mu_assert("mem97", addWNIIntoPacketForHost(NULL, 2, timestamp, notices) == NULL);
	mu_assert("mem98", packet->hostid == -1);
	mu_assert("mem99", packet->timestamp == NULL);
	mu_assert("mem100", packet->wnCount == 0);
	mu_assert("mem101", packet->wnArray[0] == -1);
	//normal add

	//return value not null	
	mu_assert("mem68", incorporateWnPacket(packet) == 0);
	mu_assert("mem69", (procArray[1].intervalList->timestamp)[1] == 1);
	mu_assert("mem70", (procArray[1].intervalList->timestamp)[0] == 0);
	mu_assert("mem71", (procArray[1].intervalList->timestamp)[2] == 0);
	mu_assert("mem72", (procArray[1].intervalList->timestamp)[3] == 0);
	mu_assert("mem73", (procArray[1].intervalList)->notices == pageArray[2].notices[1]);
	mu_assert("mem74", (procArray[1].intervalList)->notices->nextInInterval == pageArray[1023].notices[1]);
	mu_assert("mem75", (procArray[1].intervalList)->notices->nextInInterval->nextInInterval == NULL);
	mu_assert("mem76", (procArray[1].intervalList)->notices->nextInPage == NULL);
	mu_assert("mem77", (procArray[1].intervalList)->notices->nextInInterval->nextInPage == NULL);
	mu_assert("mem78", (procArray[1].intervalList)->notices->interval == procArray[1].intervalList);
	mu_assert("mem79", (procArray[1].intervalList)->notices->nextInInterval->interval == procArray[1].intervalList);
	mu_assert("mem80", (procArray[1].intervalList)->notices->diffAddress == NULL);
	mu_assert("mem81", (procArray[1].intervalList)->notices->nextInInterval->diffAddress == NULL);
	mu_assert("mem81.1", (procArray[1].intervalList)->notices->nextInInterval->pageIndex == 1023);
	mu_assert("mem81.2", (procArray[1].intervalList)->notices->pageIndex == 2);

	memset(packet, 0, sizeof(wnPacket_t));
	packet->hostid = 1;
	(packet->timestamp)[1] = 2;
	packet->wnCount = 1;
	(packet->wnArray)[0] = 2;
	
	mu_assert("mem82", incorporateWnPacket(packet) == 0);
	mu_assert("mem83", (procArray[1].intervalList->timestamp)[1] == 2);
	mu_assert("mem84", (procArray[1].intervalList->timestamp)[0] == 0);
	mu_assert("mem85", (procArray[1].intervalList->timestamp)[2] == 0);
	mu_assert("mem86", (procArray[1].intervalList->timestamp)[3] == 0);
	mu_assert("mem87", (procArray[1].intervalList->next->timestamp)[0] == 0);
	mu_assert("mem88", (procArray[1].intervalList->next->timestamp)[1] == 1);
	mu_assert("mem89", (procArray[1].intervalList->next->timestamp)[2] == 0);
	mu_assert("mem90", (procArray[1].intervalList->next->timestamp)[3] == 0);


	mu_assert("mem91", (procArray[1].intervalList)->notices == pageArray[2].notices[1]);
	mu_assert("mem92", (procArray[1].intervalList)->notices->nextInInterval == NULL);
	mu_assert("mem93", (procArray[1].intervalList)->notices->nextInPage->interval == procArray[1].intervalList->next);
	mu_assert("mem94", (procArray[1].intervalList)->notices->interval == procArray[1].intervalList);
	mu_assert("mem95", (procArray[1].intervalList)->notices->diffAddress == NULL);
	mu_assert("mem95.1", (procArray[1].intervalList)->notices->pageIndex == 2);

	return 0;
}

static char *all_tests(){
	mu_run_test(test_isAfterInterval);
	mu_run_test(test_createTwinPage);
	mu_run_test(test_freeTwinPage);
	mu_run_test(test_applyDiff);
	mu_run_test(test_createLocalDiff);
	mu_run_test(test_createWriteNotice);
	mu_run_test(test_addNewInterval);
	mu_run_test(test_incorporateWnPacket);
	mu_run_test(test_addWNIIntoPacketForHost);
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

