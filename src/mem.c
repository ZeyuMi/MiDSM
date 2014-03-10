#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "mem.h"
#include "net.h"

extern int myhostid;
extern int hostnum;

page_t pageArray[MAX_PAGE_NUM];
proc_t procArray[MAX_HOST_NUM];
interval_t *intervalNow;
int fetchPageWaitFlag, fetchDiffWaitFlag, fetchWNIWaitFlag; 
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


/**
* This procedure will send a FETCH_WN_I to hostid and it will be blocked when waiting for response.
* parameters
*	hostid : id of the destination host 
* return value
*	 0 --- success
*	-1 --- parameters error
**/
int fetchWritenoticeAndInterval(int hostid){
}


/**
* This procedure will send a GRANT_WN_I msg to 'hostid'. It will also append 'intervalNow->timestamp' to msg. All intervals which after 'timestamp' and their related writenotices will also be saved to msg.
* parameters
*	hostid    : destination host
*	timestamp : a pointer to a timestamp array.
* return value
*	 0 --- success
*	-1 --- parameters error
**/
int grantWNI(int hostid, int *timestamp){
	if(hostid < 0 || hostid >= hostnum || timestamp == NULL){
		return -1;
	}
	if(hostid == myhostid){
		return -1;
	}
	
	mimsg_t *msg = nextFreeMsgInQueue(0);
	msg->from = myhostid;
	msg->to = hostid;
	msg->command = GRANT_WN_I;
	int i;
	for(i = 0; i < MAX_HOST_NUM; i++){
		msg->timestamp[i] = intervalNow->timestamp[i];
	}
	int packetNum = 0;
	apendMsgData(msg, (char *)&packetNum, sizeof(int)); //occupy the packetNum slot
	wnPacket_t packet;
	for(i = 0; i < hostnum; i++){
		proc_t proc = procArray[i];
		interval_t *intervalLast = proc.intervalList;
		if(intervalLast == NULL){
			continue;
		}
		while(intervalLast->next != NULL){
			intervalLast = intervalLast->next;
		}
		while(intervalLast != NULL){
			if(isAfterInterval(intervalLast->timestamp, timestamp) && (intervalLast->notices != NULL)){
				writenotice_t *left = addWNIIntoPacketForHost(&packet, i, intervalLast->timestamp, intervalLast->notices);
				apendMsgData(msg, (char *)&packet, sizeof(wnPacket_t));
				packetNum++;
				while(left != NULL){
					left = addWNIIntoPacketForHost(&packet, i, intervalLast->timestamp, left);
					apendMsgData(msg, (char *)&packet, sizeof(wnPacket_t));
					packetNum++;
				}
			}
			intervalLast = intervalLast->prev;
		}
	}
	//refresh the packetNum value
	memcpy(msg->data, &packetNum, sizeof(int));
	sendMsg(msg);
	return 0;
}


/**
* This procedure will put a page into a GRANT_PAGE msg, and send it to 'hostid'.
* parameters
*	hostid    : destination host
*	pageIndex : index of a page, which will be sent to 'hostid'
* return value
*	 0 --- success
*	-1 --- parameters error
**/
int grantPage(int hostid, int pageIndex){
	if(hostid < 0 || hostid >= hostnum){
		return -1;
	}
	if(hostid == myhostid){
		return -1;
	}
	if(pageIndex < 0 || pageIndex >= MAX_HOST_NUM){
		return -1;
	}
	if(pageArray[pageIndex].state == UNMAP || pageArray[pageIndex].state == MISS){
		return -1;
	}

	mimsg_t *msg = nextFreeMsgInQueue(0);
	msg->from = myhostid;
	msg->to = hostid;
	msg->command = GRANT_PAGE;
	apendMsgData(msg, (char *)&pageIndex, sizeof(int));
	apendMsgData(msg, (char *)pageArray[pageIndex].address, PAGESIZE);
	sendMsg(msg);
	return 0;
}


/**
* This procedure will put a diff which comes from a page with index 'pageIndex' to a msg, if diff does not exist, this procedure will create one. Then, it will send a GRANT_DIFF msg to 'hostid'.
* parameters
*	hostid    : destination host
*	timestamp : a pointer to a timestamp array.
*	pageIndex : index of a page, whose diff will be sent to 'hostid'
* return value
*	 0 --- success
*	-1 --- parameters error
*	-2 --- no such page
*	
**/
int grantDiff(int hostid, int *timestamp, int pageIndex){
	if(hostid < 0 || hostid >= hostnum || hostid == myhostid){
		return -1;
	}
	if(timestamp == NULL){
		return -1;
	}
	if(pageIndex < 0 || pageIndex >= MAX_PAGE_NUM){
		return -1;
	}
	if(pageArray[pageIndex].state == UNMAP || pageArray[pageIndex].state == MISS){
		return -1;
	}
	writenotice_t *wn = pageArray[pageIndex].notices[myhostid];

	int i;
	int find = 1;
	while(wn != NULL){
		find = 1;
		for(i = 0; i < MAX_HOST_NUM; i++){
			if(wn->interval->timestamp[i] != timestamp[i]){
				find = 0;
				break;
			}
		}
		if(find == 1){
			break;
		}
		wn = wn->nextInPage;
	}
	if(wn != NULL && find == 1){//find writenotice
		if(wn->diffAddress == NULL){//diff has not been created
			wn->diffAddress = createLocalDiff(pageArray[pageIndex].address, pageArray[pageIndex].twinPage);
			freeTwinPage(pageIndex);
			pageArray[pageIndex].state = RDONLY;
			mprotect(pageArray[pageIndex].address, PAGESIZE, PROT_READ);	
		}
		mimsg_t *msg = nextFreeMsgInQueue(0);
		msg->from = myhostid;
		msg->to = hostid;
		msg->command = GRANT_DIFF;
		apendMsgData(msg, (char *)timestamp, sizeof(int) * MAX_HOST_NUM);
		apendMsgData(msg, (char *)&pageIndex, sizeof(int));
		apendMsgData(msg, wn->diffAddress, PAGESIZE);
		sendMsg(msg);
		return 0;
	}else{// not find writenotice
		return -2;
	}
}


/**
* add the memory block pointed by 'diffAddress' to the memory block pointed by 'pageAddress', the sizes of whose are PAGESIZE
* parameters
*	pageAddress : pointer of targeted memory block
*	diffAddress : pointed of twinPage
* return value
*	 0 --- success
*	-1 --- parameters error 
**/
int applyDiff(void *pageAddress, void *diffAddress){
	if(pageAddress == NULL || diffAddress == NULL){
		return -1;
	}
	int i;
	for(i = 0; i < PAGESIZE; i++){
		*((char *)pageAddress+i) = *((char *)pageAddress+i) + *((char *)diffAddress+i);
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
	interval_t *interval = procArray[hostid].intervalList;
	int find = 1;
	int i;
	while(interval != NULL){
		find = 1;
		for(i = 0; i < MAX_HOST_NUM; i++){
			if(interval->timestamp[i] != timestamp[i]){
				find = 0;
				break;
			}	
		}
		if(find == 1){
			break;
		}else{
			interval = interval->next;
		}
	}
	writenotice_t *lastwn = NULL;
	if(find == 1 && interval != NULL){
		lastwn = interval->notices;
		while(lastwn != NULL && lastwn->nextInInterval != NULL){
			lastwn = lastwn->nextInInterval;
		}	
	}else{
		interval = malloc(sizeof(interval_t));
		interval->notices = NULL;
		interval->next = NULL;
		interval->prev = NULL;
		interval->isBarrier = 0;
		for(i = 0; i < MAX_HOST_NUM; i++){
			(interval->timestamp)[i] = timestamp[i];
		}
		interval->next = procArray[hostid].intervalList;
		if(procArray[hostid].intervalList != NULL){
			procArray[hostid].intervalList->prev = interval;
		}
		procArray[hostid].intervalList = interval;
	}
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


void handleFetchWNIMsg(mimsg_t *msg){

}


/**
* This  procedure will be invoked by dispatchMsg. It will copy diff to a writenotice and apply the diff to its page. After that, this procedure will set fetchDiffWaitFlag to 0.
* parameters
*	msg : msg to be handled
**/
void handleGrantDiffMsg(mimsg_t *msg){
	if(msg == NULL){
		return;
	}
	int hostid = msg->from;
	int *timestamp = (int *)msg->data; 
	int pageIndex = *((int *)(msg->data + sizeof(int) * MAX_HOST_NUM));
	void *diffAddress = msg->data + sizeof(int) * (MAX_HOST_NUM + 1);
	
	int i;
	int find = 1;
	writenotice_t *wn = pageArray[pageIndex].notices[hostid];
	while(wn != NULL){
		find = 1;
		for(i = 0; i < MAX_HOST_NUM; i++){
			if(wn->interval->timestamp[i] != timestamp[i]){
				find = 0;
				break;
			}
		}
		if(find == 1){
			break;
		}
		wn = wn->nextInPage;
	}
	if(wn != NULL && find == 1){//find writenotice
		if(wn->diffAddress == NULL){//diff has not been created
			wn->diffAddress = malloc(PAGESIZE);
			memset(wn->diffAddress, 0, PAGESIZE);

			bcopy(diffAddress, wn->diffAddress, PAGESIZE);
		}
		applyDiff(pageArray[pageIndex].address, diffAddress);
	}
	fetchDiffWaitFlag = 0;
	return;
}


/**
* This procedure will be invoked by dispatchMsg. It will save page content from msg.
* parameters
*	msg : msg to be handled  
**/
void handleGrantPageMsg(mimsg_t *msg){
	if(msg == NULL){
		return;
	}
	int pageIndex = *((int *)msg->data);
	void *pageAddress = msg->data + sizeof(int);
	mmap(pageArray[pageIndex].address, PAGESIZE, PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_FIXED, mapfd, 0);
	bcopy(pageAddress, pageArray[pageIndex].address, PAGESIZE);
	fetchPageWaitFlag = 0;
}


/**
* This procedure will be invoked by dispatchMsg. It will incoporates all wnPacket_t in 'msg' to local data structure.
* parameters
*	msg : msg to be handled  
**/
void handleGrantWNIMsg(mimsg_t *msg){
	if(msg == NULL){
		return;
	}	
	intervalNow = malloc(sizeof(interval_t));
	memset(intervalNow, 0, sizeof(interval_t));

	int i;
	for(i = 0; i < MAX_HOST_NUM; i++){
		intervalNow->timestamp[i] = msg->timestamp[i];
	}
	(intervalNow->timestamp[myhostid])++;
	intervalNow->next = procArray[myhostid].intervalList;
	if(procArray[myhostid].intervalList != NULL){
		procArray[myhostid].intervalList->prev = intervalNow;
	}
	procArray[myhostid].intervalList = intervalNow;
	
	int packetNum = *((int *)msg->data);
	wnPacket_t *packet =(wnPacket_t *)(msg->data + sizeof(int));
	for(i = 0; i < packetNum; i++){
		packet = packet + i;	
		incorporateWnPacket(packet);
	}
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
	intervalNow->prev = NULL;
	if(procArray[myhostid].intervalList != NULL){
		procArray[myhostid].intervalList->prev = intervalNow;
	}
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
	if(packet == NULL || timestamp == NULL || notices == NULL){
		return NULL;
	}
	if(hostid < 0 || hostid >= hostnum){
		return NULL;
	}
	packet->hostid = -1;
	memset(packet->timestamp, 0, MAX_HOST_NUM * sizeof(int));
	packet->wnCount = 0;
	int i;
	for(i = 0; i < MAX_WN_NUM; i++){
		packet->wnArray[i] = -1;
	}
	packet->hostid = hostid;
	for(i = 0; i < MAX_HOST_NUM; i++){
		packet->timestamp[i] = timestamp[i];
	}
	while((notices != NULL) && (packet->wnCount < MAX_WN_NUM)){
		packet->wnArray[packet->wnCount] = notices->pageIndex;
		(packet->wnCount)++;
		notices = notices->nextInInterval;
	}
	if(notices != NULL){
		return notices;
	}else{
		return NULL;
	}
}

