#ifndef MISYN_H
#define MISYN_H

#include "net.h"

#define LOCK_NUM 1024

typedef struct miAcquirer{
		int hostid;
		struct miAcquirer *next;
	}acquirer_t;
typedef struct {
		int state;
		int lasthostid;
		acquirer_t *waitingList;
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
int freeLock(int lockno);
int checkBarrierFLags();
#endif
