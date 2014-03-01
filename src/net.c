#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>
#include "net.h"

mimsg_t sndQueue[MAX_QUEUE_SIZE];
mimsg_t recvQueue[MAX_QUEUE_SIZE];
int sndHead, sndTail, sndQueueSize, recvHead, recvTail, recvQueueSize;

void sigio_handler(int sigio, siginfo_t *info, void context){

}


/**
* initialization of net module
**/
void initnet(){
	/***************initialize global variables*******************/	
	sndHead = sndTail = sndQueueSize = recvHead = recvTail = recvQueueSize = 0;
	/***************set up signal handler*************************/
	struct sigaction act;
	act.sa_handler = (void (*)(int))sigio_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGIO, &act, NULL);
	/***************initialize port number************************/
	
	/***************create sockets********************************/
}


/**
* create DGRAM socket with ip = INADDR_ANY, port = 'port' if isRecv = '1'; set the sizes of sndBuffer and recvBuffer = 'bufSize'
* parameters
*	port    : the port of a socket
*	isRecv  : '1' indicates that the socket does not need its port specified; '0' means not setting the port
*	bufSize : the size of sndBuffer and recvBuffer
* return value
*	>= 0 --- the file descriptor of the socket
*	  -1 --- parameters error
**/
int createSocket(short int port, int isRecv, int bufSize){
	if(port <= 1024 || (isRecv != 0 && isRecv != 1) || bufSize <= 0){
		return -1;
	}
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	if(isRecv == 1){
		addr.sin_port = htons(port);
	}else{
		addr.sin_port = htons(0);
	}
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&bufSize, sizeof(bufSize));
	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&bufSize, sizeof(bufSize));
	
	return fd;
}


/**
* copy a mimsg_t into the tail of sndQueue or recvQueue which is specified by the first parameter, type
* parameters
*	type : 0 indicates sndQueue; 1 indicates recvQueue
*	msg  : a pointer to the mimsg_t
* return value
*	 0 --- success
*	-1 --- parameters error
*	-2 --- queue is full
**/
int msgEnqueue(int type, mimsg_t *msg){
	if(sndQueueSize == MAX_QUEUE_SIZE){
		return -2;
	}
	if((type != 0 && type != 1) || msg == NULL){
		return -1;	
	}
	if(type == 0){
		memcpy(&(sndQueue[sndTail]), msg, sizeof(mimsg_t));
		sndTail = (sndTail + 1) % MAX_QUEUE_SIZE;	
		sndQueueSize++;
	}else{
		memcpy(&(recvQueue[recvTail]), msg, sizeof(mimsg_t));
		recvTail = (recvTail + 1) % MAX_QUEUE_SIZE;	
		recvQueueSize++;
	}
	return 0;
}


/** 
* according to the paramter type, return the pointer to the head of a queue* parameters
*	type : 0 indicates sndQueue; 1 indicates recvQueue
* return value
*	NOT NULL --- success
*	NULL     --- queue is empty or the value of type is wrong
**/
mimsg_t *queueTop(int type){
	if(type != 0 && type != 1){
		return NULL;
	}
	if(type == 0){
		if(sndQueueSize == 0){
			return NULL;
		}else{
			return &(sndQueue[sndHead]);
		}
	}else{
		if(recvQueueSize == 0){
			return NULL;
		}else{
			return &(recvQueue[recvHead]);
		}
	}
}


/**
* according to the paramter type, destory the msg on the top of the queue
* parameters
*	type : 0 indicates sndQueue; 1 indicates recvQueue
* return value
*	 0 --- success
*	-1 --- parameters error
*	-2 --- queue is empty
**/
int msgDequeue(int type){
	if(type != 0 && type != 1){
		return -1;
	}
	if(type == 0){
		if(sndQueueSize == 0){
			return -2;	
		}else{
			sndHead = (sndHead + 1) % MAX_QUEUE_SIZE;
			sndQueueSize--;
		}
	}else{
		if(recvQueueSize == 0){
			return -2;	
		}else{
			recvHead = (recvHead + 1) % MAX_QUEUE_SIZE;
			recvQueueSize--;
		}
	}
	return 0;
}


int sendMsg(mimsg_t *msg){


}


/**
* apend data whose length is 'len' to the end of msg->data
* parameters
*	msg  : pointer to the mimsg_t
*	data : pointer to the data to be apended
* return value 
*	 0 --- success
*	-1 --- parameters error
*	-2 --- len is too large
**/
int apendMsgData(mimsg_t *msg, char *data, int len){
	if(msg == NULL || data == NULL || len <= 0){
		return -1;
	}
	if(len > MAX_MSG_SIZE - MSG_HEAD_SIZE){
		return -2;
	}
	memcpy(&(msg->data[msg->size]), data, len);
	msg->size += len;
	return 0;
}


/**
* create a new msg and return its pointer
* return value
*	NOT NULL --- success
*	NULL     --- malloc failure
**/
mimsg_t *newMsg(){
	mimsg_t *msg = malloc(sizeof(mimsg_t));
	if(msg == NULL){
		return NULL;
	}	
	memset((void *)msg, 0, sizeof(mimsg_t));	
	msg->from = -1;
	msg->to = -1;
	msg->command = -1;
	msg->size = 0;
	msg->seqno = 0;
	return msg;
}


/**
* free the memory area pointed by a mimsg_t pointer
* parameters
*	msg : pointing to the area to be freed
* return value
*	 0 --- success
*	-1 --- msg == NULL 	
**/
int freeMsg(mimsg_t *msg){
	if(msg == NULL){
		return -1;
	}else{
		free(msg);
		return 0;
	}	
}
