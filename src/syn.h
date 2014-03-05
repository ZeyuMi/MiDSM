#ifndef MISYN_H
#define MISYN_H

#include "net.h"
#include "init.h"

#define LOCK_NUM 1024

typedef struct {
		int state;
		int owner;
		int lasthostid;
		int waitingList[MAX_HOST_NUM];
		int waitingListCount;
	} milock_t;

typedef enum {LOCKED, FREE} lockstate_t;

void initsyn();
int mi_lock(int lockno);
int mi_unlock(int lockno);
void mi_barrier();
void handleAcquireMsg(mimsg_t *msg);
void handleReleaseMsg(mimsg_t *msg);
void handleEnterBarrierMsg(mimsg_t *msg);
void handleExitBarrierMsg(mimsg_t *msg);
void handleGrantMsg(mimsg_t *msg);
int graspLock(int lockno, int hostid);
void grantLock(int lockno, int hostid);
int freeLock(int lockno, int hostid);
int checkBarrierFlags();
#endif
