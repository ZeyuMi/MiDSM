#include <stdio.h>
#include <stdlib.h>
#include "syn.h"

milock_t locks[LOCK_NUM];
int waitFlag;
int myLocks[LOCK_NUM];
int barrierFLags[MAX_HOST_NUM];


/**
* initialization of syn module, set initial values for all global variables
**/
void initsyn(){
	int i;
	for(i = 0; i < LOCK_NUM; i++){
		locks[i].state = FREE;
		locks[i].lasthostid = -1;
		locks[i].waitingList = NULL;
	}
	waitFlag = 0;
	for(i = 0; i < LOCK_NUM; i++){
		myLocks[i] = 0;
	}
	for(i = 0; i < MAX_HOST_NUM; i++){
		barrierFLags[i] = 0;
	}
}


int mi_lock(int lockno){


}


int mi_unlock(int lockno){

}


void mi_barrier(){

}


void handleAcquireMsg(mimsg_t *msg){

}


void handleReleaseMsg(mimsg_t *msg){

}


void handleEnterBarrierMsg(mimsg_t *msg){

}


void handleExitBarrierMsg(mimsg_t *msg){

}


void handleGrantMsg(mimsg_t *msg){

}


int graspLock(int lockno, int hostid){

}


void grantLock(int lockno, int hostid){

}


int freeLock(int lockno, int hostid){

}


int checkBarrierFLags(){

}

