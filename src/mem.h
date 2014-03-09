#ifndef MIMEM_H
#define MIMEM_H

#include "init.h"
#include "net.h"

#define PAGESIZE 4096
#define MAX_MEM_SIZE 0x08000000
#define MAX_PAGE_NUM (MAX_MEM_SIZE / PAGESIZE) 
#define START_ADDRESS 0x60000000
#define MAX_WN_NUM 1024

typedef enum {UNMAP, RDONLY, WRITE, MISS, INVALID} pagestate_t;
typedef struct interval{
		int timestamp[MAX_HOST_NUM];
		struct writenotice *notices;
		struct interval *next;
		int isBarrier;
	}interval_t;
typedef struct writenotice{
		interval_t *interval;
		struct writenotice *nextInPage;
		struct writenotice *nextInInterval;
		void *diffAddress;
	}writenotice_t;
typedef struct {
		void *address;
		int state;
		writenotice_t *notices[MAX_HOST_NUM];
		void *twinPage;
	}page_t;
typedef struct {
		int hostid;
		interval_t *intervalList;
	}proc_t;
typedef struct {
		int hostid;
		int timestamp[MAX_HOST_NUM];
		int wnCount;
		int wnArray[MAX_WN_NUM];
	}wnPacket_t;

void init_mem();
void *mi_alloc(int size);
int fetchPage(int pageIndex);
int createTwinPage(int pageIndex);
int freeTwinPage(int pageIndex);
int fetchDiff(int pageIndex);
int fetchWritenoticeAndInterval(int hostid);
int grantWNI(int hostid, int *timestamp);
int grantPage(int hostid, int pageIndex);
int grantDiff(int hostid, int *timestamp, int pageIndex);
int applyDiff(void *destAddress, void *diffAddress);
void *createLocalDiff(void *pageAddress, void *twinAddress);
int incorporateWnPacket(wnPacket_t *packet);
int createWriteNotice(int pageIndex);
void handleFetchPageMsg(mimsg_t *msg);
void handleFetchDiffMsg(mimsg_t *msg);
void handleGrantDiffMsg(mimsg_t *msg);
void handleGrantPageMsg(mimsg_t *msg);
void handleGrantWNIMsg(mimsg_t *msg);
int isAfterInterval(int *timestamp, int *targetTimestamp);
void addNewInterval();
writenotice_t *addWNIIntoPacketForHost(wnPacket_t *packet, int hostid, int *timestamp, writenotice_t *notices);
#endif
