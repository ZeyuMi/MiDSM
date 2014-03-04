#include <stdio.h>
#include <stdlib.h>
#include "syn.h"

extern int myhostid;
extern int hostnum;

milock_t locks[LOCK_NUM];
int waitFlag;
int myLocks[LOCK_NUM];
int barrierFlags[MAX_HOST_NUM];


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
		barrierFlags[i] = 0;
	}
}


/** 
* 1
**/
int mi_lock(int lockno){


}


/**
* 2
**/
int mi_unlock(int lockno){

}


/** 
* 3
**/
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


/**
* lock manager will use this procedure to check lock state. If lock is free, return 0, otherwise return -3, and add hostid to the lock waiting list.
* parameters
*	lockno : index of lock in locks array
*	hostid : host which want to acquire this lock
* return value
*	 0 --- success
*	-1 --- parameters error
*	-2 --- this node has no right to manipulate this lock
*	-3 --- lock is busy
**/
int graspLock(int lockno, int hostid){
	extern hostnum;
	extern myhostid;
	if(lockno < 0 || lockno >= LOCK_NUM || hostid < 0 || hostid >= hostnum){
		return -1;
	}
	if((lockno % hostnum) != myhostid){
		return -2;
	}
	if(locks[lockno].state == FREE){
		locks[lockno].state = LOCKED;
		return 0;
	}else{
		acquirer_t *acquirer = malloc(sizeof(acquirer_t));
		acquirer->next = NULL;
		acquirer->hostid = hostid;
		if(locks[lockno].waitingList == NULL){
			locks[lockno].waitingList = acquirer;
		}else{
			acquirer_t *temp = locks[lockno].waitingList;
			while(temp->next != NULL){
				temp = temp->next;
			}
			temp->next = acquirer;
		}
		return -3;
	}
}


/** 
* send GRANT msg to hostid, telling it that lockno belongs to it
* parameters
*	lockno : index of lock in locks array
*	hostid : the id of host which will be received GRANT msg
**/
void grantLock(int lockno, int hostid){
	if((lockno < -1) || (lockno > 1023) || (hostid < 0) || (hostid >= hostnum)){
		return;
	}
	if(hostid == myhostid){
		return;
	}
	if((lockno % hostnum) != myhostid){
		return;
	}
	mimsg_t *msg = nextFreeMsgInQueue(0);
	msg->from = myhostid;
	msg->to = hostid;
	msg->command = GRANT;
	char buffer[20];
	sprintf(buffer, "%d", lockno);
	apendMsgData(msg, buffer, sizeof(int));
	sendMsg(msg);
}


/**
* lock manager will use this procedure to change lock state to free. If waiting list of this lock is not NULL, the top hostid of waiting list will be returned.
* parameters
*	lockno : index of lock in locks array
* return value
*	>= 0 --- id of a waiting host
*	  -1 --- parameters error
*	  -2 --- this node has no right to manipulate this lock
*	  -3 --- the state of this lock is free
*	  -4 --- no waiting host
**/
int freeLock(int lockno){
	extern hostnum;
	extern myhostid;
	if(lockno < 0 || lockno >= LOCK_NUM){
		return -1;
	}
	if((lockno % hostnum) != myhostid){
		return -2;
	}
	if(locks[lockno].state == FREE){
		return -3;
	}else{
		if(locks[lockno].waitingList == NULL){
			locks[lockno].state = FREE;
			return -4;
		}else{
			int waitinghostid = locks[lockno].waitingList->hostid;
			locks[lockno].waitingList = locks[lockno].waitingList->next;
			return waitinghostid;
		}
	}
}


/**
* if all hosts enter barrier, send msg to them which tells them exit barrier.
* return value
*	 0 --- success
*	-1 --- not all elements of barrierFlags are 1    
*	-2 --- this node has no right to manipulate barriers
**/
int checkBarrierFlags(){
	if(myhostid != 0){
		return -2;
	}
	int i;
	for(i = 0; i < hostnum; i++){
		if(barrierFlags[i] != 1){
			return -1;
		}
	}
	mimsg_t *msg;
	for(i = 1; i < hostnum; i++){
		msg = nextFreeMsgInQueue(0);
		msg->from = 0;
		msg->to = i;
		msg->command = EXIT_BARRIER;
		sendMsg(msg);
	}
	return 0;
}

