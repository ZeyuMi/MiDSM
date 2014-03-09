#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "mem.h"

extern int myhostid;
extern int hostnum;

page_t pageArray[MAX_PAGE_NUM];
proc_t procArray[MAX_HOST_NUM];
interval_t *intervalNow;
int fetchPageWaitFlag, fetchDiffWaitFlag; 
int pagenum;
long mapfd;
long globalAddress;

void segv_handler(int signo, siginfo_t *info, void *context){
	void *faultaddress = info->si_addr;
	void *addr = ((long)faultaddress % PAGESIZE == 0) ? faultaddress : (void *)((long)faultaddress / PAGESIZE * PAGESIZE) ;
	/*check whether addr is in valid range*/
	void *maxaddr = pageArray[pagenum-1].address + PAGESIZE;
	if(addr > maxaddr){
		printf("invalid address\n");
		exit(1);
	}
	int pageindex = ((long)addr - START_ADDRESS) / PAGESIZE;
	if(pageArray[pageindex].state == RDONLY){
		pageArray[pageindex].state = WRITE;
		if(mprotect(pageArray[pageindex].address, PAGESIZE, PROT_READ | PROT_WRITE) == -1)
			printf("error\n");
		/*create twin page*/
		printf("read to write\n");


	}else if(pageArray[pageindex].state == MISS){

	}	
}


void *mi_alloc(int size){
	int rsize = (size % PAGESIZE == 0) ? size : (size / PAGESIZE + 1) * PAGESIZE;
	int allocesize = rsize;
	if(rsize + globalAddress > MAX_MEM_SIZE)
		return NULL;
	while(allocesize > 0){
		void *a = mmap((void *)(START_ADDRESS + globalAddress), 4096, PROT_READ , MAP_PRIVATE | MAP_FIXED, mapfd, 0);	
		printf("%p\n", a);
		pageArray[pagenum].address = (void *)START_ADDRESS + globalAddress;
		pageArray[pagenum].state = RDONLY;
		pagenum++;
		allocesize -= PAGESIZE;
	}
	globalAddress += rsize;
	return (void *)(START_ADDRESS + globalAddress - rsize);
}


/**
* initialization work of memory management before the system is normally used 
**/
void init_mem(){
/*******************install segv signal handler*************/
	struct sigaction act;
	act.sa_handler = (void (*)(int))segv_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &act, NULL);
/*******************initialize pages************************/
	int i;
	for(i = 0; i < MAX_PAGE_NUM; i++){
		pageArray[i].address = 0;
		pageArray[i].state = 0;	
	}
/*******************prepare mapped file*********************/	
	mapfd = open("/dev/zero", O_RDWR, 0);
/*******************initialize global variables*********************/	
	globalAddress = 0;
	pagenum = 0;
}


int fetchPage(int pageIndex){

}


/**
* create twin page for page in pageArray with index 'pageIndex', and store address of twin page in pageArray['pageIndex'].twinPage
* parameters
*	pageIndex : index in pageArray
* return value
*	 0 --- success
*	-1 --- parameter error
**/
int createTwinPage(int pageIndex){
	if(pageIndex < 0 || pageIndex >= MAX_PAGE_NUM){
		return -1;
	}
	if(pageArray[pageIndex].state == UNMAP || pageArray[pageIndex].state == MISS){
		return -1;
	}
	if(pageArray[pageIndex].address == NULL || pageArray[pageIndex].twinPage != NULL){
		return -1;
	}
	pageArray[pageIndex].twinPage = malloc(PAGESIZE);
	bcopy(pageArray[pageIndex].address, pageArray[pageIndex].twinPage, PAGESIZE);
	return 0;
}


/**
* free the memory area that arrayPage['pageIndex'].twinPage points to
* parameters
*	pageIndex : index in pageArray
* return value
*	 0 --- success
*	-1 --- parameters error
**/
int freeTwinPage(int pageIndex){
	if(pageIndex < 0 || pageIndex >= MAX_PAGE_NUM){
		return -1;
	}
	if(pageArray[pageIndex].state == UNMAP || pageArray[pageIndex].state == MISS){
		return -1;
	}
	if(pageArray[pageIndex].address == NULL || pageArray[pageIndex].twinPage == NULL){
		return -1;
	}
	free(pageArray[pageIndex].twinPage);
	pageArray[pageIndex].twinPage = NULL;
	return 0;
}


int fetchDiff(int pageIndex){

}


int fetchWritenoticeAndInterval(int hostid){

}


int grantWNI(int hostid, int *timestamp){

}


int grantPage(int hostid, int pageIndex){

}


int grantDiff(int hostid, int *timestamp, int pageIndex){

}


/**
* add the memory block pointed by 'twinAddress' to the memory block pointed by 'pageAddress', the sizes of whose are PAGESIZE
* parameters
*	pageAddress : pointer of targeted memory block
*	twinAddress : pointed of twinPage
* return value
*	 0 --- success
*	-1 --- parameters error 
**/
int applyDiff(void *pageAddress, void *twinAddress){
	if(pageAddress == NULL || twinAddress == NULL){
		return -1;
	}
	int i;
	for(i = 0; i < PAGESIZE; i++){
		*((char *)pageAddress+i) = *((char *)pageAddress+i) + *((char *)twinAddress+i);
	}
	return 0;
}


/**
* allocate a new memory block whose size is equal to PAGESIZE, and save a value into the 'i'th element which is the result of 'i'th element in 'pageAddress' minus 'i'the element in 'twinAddress'.  
* parameters
*	pageAddress : pointer of a memory block
*	twinAddress : pointer of a memory block
* return value
*	NOT NULL --- success
*	NULL     --- parameters error
**/
void *createLocalDiff(void *pageAddress, void *twinAddress){
	if(pageAddress == NULL || twinAddress == NULL){
		return NULL;
	}
	void *diffAddress = malloc(PAGESIZE);
	int i;
	for(i = 0; i < PAGESIZE; i++){
		*((char *)diffAddress+i) = *((char *)pageAddress+i) - *((char *)twinAddress+i);
	}
	return diffAddress;

}


/**
* This procedure incorporates an interval and its writenotices into local data structures. The page which received writenotice will become 'INVALID' and create local diff.
* parameters
	packet : a pointer to the struct that encapsulates interval and writenotices
* return value
*	 0 --- success
*	-1 --- parameters error
**/
int incorporateWnPacket(wnPacket_t *packet){
	if(packet == NULL){
		return -1;
	}
	int hostid = packet->hostid;
	int *timestamp = packet->timestamp;
	int wnCount = packet->wnCount;
	interval_t *interval = malloc(sizeof(interval_t));
	interval->notices = NULL;
	interval->next = NULL;
	interval->isBarrier = 0;
	int i;
	for(i = 0; i < MAX_HOST_NUM; i++){
		(interval->timestamp)[i] = timestamp[i];
	}
	interval->next = procArray[hostid].intervalList;
	procArray[hostid].intervalList = interval;
	writenotice_t *lastwn = NULL;
	for(i = 0; i < wnCount; i++){
		int pageIndex = (packet->wnArray)[i];
		writenotice_t *wn = malloc(sizeof(writenotice_t));
		wn->interval = interval;
		wn->nextInPage = pageArray[pageIndex].notices[hostid];
		pageArray[pageIndex].notices[hostid] = wn;
		wn->diffAddress = NULL;
		wn->nextInInterval = NULL;
		wn->pageIndex = pageIndex;
		if(lastwn == NULL){
			interval->notices = wn;
		}else{
			lastwn->nextInInterval = wn;
		}
		lastwn = wn;
	}
	return 0;
}


/**
* create a local writenotice for a page element in pageArray which is indexed by 'pageIndex'
* parameters
*	pageIndex : index in pageArray
* return value
*	 0 --- success
*	-1 --- parameters error 
**/
int createWriteNotice(int pageIndex){
	if(pageIndex < 0 || pageIndex >= MAX_PAGE_NUM){
		return -1;
	}
	if(pageArray[pageIndex].state != RDONLY){
		return -1;
	}
	
	writenotice_t *wn = malloc(sizeof(writenotice_t));
	wn->interval = intervalNow;
	wn->nextInPage = NULL;
	wn->nextInInterval = NULL;
	wn->diffAddress = NULL;
	wn->pageIndex = pageIndex;

	wn->nextInPage = pageArray[pageIndex].notices[myhostid];
	pageArray[pageIndex].notices[myhostid] = wn;
	writenotice_t *temp = intervalNow->notices;
	if(temp == NULL){
		intervalNow->notices = wn;
	}else{
		while(temp->nextInInterval != NULL){
			temp = temp->nextInInterval;	
		}
		temp->nextInInterval = wn;
	}
	return 0;
}


void handleFetchPageMsg(mimsg_t *msg){


}


void handleFetchDiffMsg(mimsg_t *msg){

}


void handleGrantDiffMsg(mimsg_t *msg){

}


void handleGrantPageMsg(mimsg_t *msg){


}


void handleGrantWNIMsg(mimsg_t *msg){

}


/**
* compare two vector timestamps, if any element in 'timestamp' is bigger than the corresponding element of 'targetTimestamp', this procedure will return 1, otherwise return 0
* parameters
*	timestamp : an int array with size equal to MAX_HOST_NUM
*	targetTimestamp : an int array with size equal to MAX_HOST_NUM
* return value
*	 0 --- negative
*	 1 --- positive
*	-1 --- parameters error
**/
int isAfterInterval(int *timestamp, int *targetTimestamp){
	if((timestamp == NULL) || (targetTimestamp == NULL)){
		return -1;
	}
	int i;
	for(i = 0; i < hostnum; i++){
		if(timestamp[i] > targetTimestamp[i]){
			return 1;
		}
	}
	return 0;
}


/**
* This procedure will be invoked when releasing a lock. It will make a new interval and point it using 'intervalNow'
**/
void addNewInterval(){
	intervalNow = malloc(sizeof(interval_t));
	memset(intervalNow, 0, sizeof(interval_t));
	int i;
	for(i = 0; i < hostnum; i++){
		(intervalNow->timestamp)[i] = (procArray[myhostid].intervalList->timestamp)[i];
	}
	(intervalNow->timestamp)[myhostid]++;
	intervalNow->notices = NULL;
	intervalNow->next = procArray[myhostid].intervalList;
	procArray[myhostid].intervalList = intervalNow;
	intervalNow->isBarrier = 0;
}


/**
* This procedure will initialize a wnPacket with hostid, timestamp and related writenotices. If the number of notices is bigger than MAX_WN_NUM, the writenotice which is not be added to the wnPacket will be returned.
* parameters
*	packet 	   : a pointer to the wnPacket which will be initialized
*	hostid	   : id of a host
*	timestamp  : a pointer to a vector timestamp
*	notices    : writenotice linked list
* return value
*	 NOT NULL --- the pointer to the writenotice linked list that is not be added to the wnPacket
*	     NULL --- success
**/
writenotice_t *addWNIIntoPacketForHost(wnPacket_t *packet, int hostid, int *timestamp, writenotice_t *notices){

}

