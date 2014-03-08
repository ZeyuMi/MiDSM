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
* 1
**/
int applyDiff(void *pageAddress, void *twinAddress){

}


/**
* 2
**/
void createLocalDiff(void *pageAddress, void *twinAddress){


}


int incorporateWnPacket(wnPacket_t *packet){

}


/**
* 3
**/
int createWriteNotice(int pageIndex){

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


int addNewInterval(){


}


writenotice_t *addWNIIntoPacketForHost(wnPacket_t *packet, int hostid, int *timestamp, writenotice_t *notices){


}

