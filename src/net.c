#include "net.h"
#include "util.h"
#include "syn.h"
#include "mem.h"

extern int myhostid;
extern int hostnum;
extern host_t hosts[MAX_HOST_NUM];

mimsg_t sndQueue[MAX_QUEUE_SIZE];
mimsg_t recvQueue[MAX_QUEUE_SIZE];
int dataPorts[MAX_HOST_NUM][MAX_HOST_NUM];
int ackPorts[MAX_HOST_NUM][MAX_HOST_NUM];
int sndHead, sndTail, sndQueueSize, recvHead, recvTail, recvQueueSize;
netmanager datamanager;
netmanager ackmanager;
int isSigioDisabled = 0;

void disableSigio(){
	sigset_t blset;
	sigemptyset(&blset);
	sigaddset(&blset, SIGIO);
	sigprocmask(SIG_BLOCK, &blset, NULL);
	isSigioDisabled = 1;
}

void enableSigio(){
	sigset_t blset;
	sigemptyset(&blset);
	sigaddset(&blset, SIGIO);
	sigprocmask(SIG_UNBLOCK, &blset, NULL);
	isSigioDisabled = 0;
}


void testCommand(mimsg_t *msg){
	printf("seqno %d : msg from %d to %d with command %d received!\n", msg->seqno, msg->from, msg->to, msg->command);
}

void sigio_handler(int sigio, siginfo_t *info, void *context){
//	printf("entering into sigio_handler\n");
//	printf("before block\n");
//	disableSigio();
	fd_set readset = datamanager.recv_fdset;
	struct timeval polltime;
	polltime.tv_sec = 0;
	polltime.tv_usec = 0;
//	printf("before select\n");
	int num = select(datamanager.recv_maxfd, &readset, NULL, NULL, &polltime);
//	printf("after select\n");
//	printf("select num %d \n", num);
	if(num > 0){
		int i;
		for(i = 0; i < MAX_HOST_NUM; i++){
			if(i != myhostid){
				//printf("receive for %d \n", i);
//				printf("before read recv_fds\n");
				int fd = datamanager.recv_fds[i];
//				printf("after read recv_fds\n");
				if(FD_ISSET(fd, &readset)){
//					printf("entering fd_isset\n");
					mimsg_t *msg = nextFreeMsgInQueue(1);
//					printf("nextFreeMsgInQueue successfully\n");
					struct sockaddr_in addr;	
					int s = sizeof(addr);
//					printf("before recvFrom\n");
					int size = recvfrom(fd, msg, sizeof(mimsg_t), 0,(struct sockaddr *) &addr, &s);
					if(size == -1){
						printf("err1: %s\n", strerror(errno));
					}


//					printf("after recvFrom size = %d\n", size);
					

					int seqno = msg->seqno;
					printf("seqno = %d\n", seqno);
					if(size > 0){
						msgEnqueue(1);	
						(datamanager.recv_seqs[i])++;
						struct sockaddr_in dest;
						dest.sin_family = AF_INET;
						inet_pton(AF_INET, hosts[i].address, &(dest.sin_addr.s_addr));
						dest.sin_port = ackPorts[msg->from][myhostid];
						sendto(ackmanager.snd_fds[i], &(msg->seqno), 4, 0, (struct sockaddr *)&dest, sizeof(dest));
//						printf("send ack %d to ip = %s, port num = %d\n", msg->seqno, hosts[i].address, ackPorts[msg->from][msg->to]);
					}
				}
			}
		}
		readset = datamanager.recv_fdset;
		polltime.tv_sec = 0;
		polltime.tv_usec = 0;
		num = select(datamanager.recv_maxfd, &readset, NULL, NULL, &polltime);
	}
//	printf("after unblock\n");
	while(recvQueueSize > 0){
		mimsg_t *msg = queueTop(1);
		dispatchMsg(msg);
		msgDequeue(1);
	}
	//enableSigio();
	
}


/**
* initialization of net module
**/
void initnet(){
	/***************initialize global variables*******************/	
	sndHead = sndTail = sndQueueSize = recvHead = recvTail = recvQueueSize = 0;
	memset(&datamanager, 0 , sizeof(netmanager));
	memset(&ackmanager, 0 , sizeof(netmanager));
	/***************set up signal handler*************************/
	struct sigaction act;
	act.sa_handler = (void (*)(int))sigio_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGIO, &act, NULL);
	/***************initialize port number************************/
	int i, j;
	for(i = 0; i < MAX_HOST_NUM; i++){
		for(j = 0; j < MAX_HOST_NUM; j++){
			dataPorts[i][j] = BASEPORT + (i+1) * MAX_HOST_NUM + j+1;
			ackPorts[i][j] = BASEPORT + (i+1) * MAX_HOST_NUM * MAX_HOST_NUM + (j+1) * MAX_HOST_NUM;
		}
	}	
	/***************create sockets********************************/	
	//create sockets for sending and receiving data
	int bufferSizeForData = (MAX_MSG_SIZE + 128) * 16;
	int bufferSizeForAck = 4;
	for(i = 0; i < MAX_HOST_NUM; i++){
		int fd = createSocket(dataPorts[myhostid][i], 1, bufferSizeForData);
		datamanager.recv_fds[i] = fd;
		datamanager.recv_maxfd = max(datamanager.recv_maxfd, fd+1);
		FD_SET(fd, &(datamanager.recv_fdset)); 
		
		int res = fcntl(fd, F_SETOWN, getpid());
		if(res == -1){
			printf("cannot setown for fd %d\n", fd);
		}
		res = fcntl(fd, F_SETFL, O_ASYNC);
		if(res == -1){
			printf("cannot set asynchronous for fd %d\n", fd);
		}
		
		printf("initialization datamanager recv_fds[%d] = %d, port = %d\n", i, fd, dataPorts[myhostid][i]);


		fd = createSocket(-1, 0, bufferSizeForData);
		datamanager.snd_fds[i] = fd;	

		
		printf("initialization datamanager snd_fds[%d] = %d\n", i, fd);


	}
	
	//create sockets for sending and receiving ACK flag
	for(i = 0; i < MAX_HOST_NUM; i++){
		int fd = createSocket(ackPorts[myhostid][i], 1, bufferSizeForAck);
		ackmanager.recv_fds[i] = fd;
		ackmanager.recv_maxfd = max(ackmanager.recv_maxfd, fd+1);
		FD_SET(fd, &ackmanager.recv_fdset); 
		
		printf("initialization ackmanager recv_fds[%d] = %d, port = %d\n", i, fd, ackPorts[myhostid][i]);


		fd = createSocket(-1, 0, bufferSizeForAck);
		ackmanager.snd_fds[i] = fd;	


		printf("initialization ackmanager snd_fds[%d] = %d\n", i, fd);


	}
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
	if((port >= 0 && port < 1024) || (isRecv != 0 && isRecv != 1) || bufSize <= 0){
		return -1;
	}

	int fd = socket(AF_INET, SOCK_DGRAM, 0);

	if(fd == -1){
		printf("err: %s\n", strerror(errno));
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	if(isRecv == 1){
		addr.sin_port = port;
	}else{
		addr.sin_port = 0;
	}
	addr.sin_addr.s_addr = INADDR_ANY;

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
int msgEnqueue(int type){
	if((type != 0) && (type != 1)){
		return -1;	
	}
	if(type == 0){
		if(sndQueueSize == MAX_QUEUE_SIZE){
			return -2;
		}else{
			sndTail = (sndTail + 1) % MAX_QUEUE_SIZE;	
			sndQueueSize++;
		}
	}else{
		if(recvQueueSize == MAX_QUEUE_SIZE){
			return -2;
		}else{
			recvTail = (recvTail + 1) % MAX_QUEUE_SIZE;	
			recvQueueSize++;
		}
	}
	return 0;
}


/** 
* according to the paramter type, return the pointer to the free element after tail in a queue
* parameters
*	type : 0 indicates sndQueue; 1 indicates recvQueue
* return value
*	NOT NULL --- success
*	NULL     --- queue is full or the value of type is wrong
*
**/
mimsg_t *nextFreeMsgInQueue(int type){
	if((type != 0) && (type != 1)){
		return NULL;
	}
	if(type == 0){
		if(sndQueueSize == MAX_QUEUE_SIZE){
			return NULL;
		}else{
			memset(&(sndQueue[sndHead]), 0, sizeof(mimsg_t));
			sndQueue[sndHead].from = -1;
			sndQueue[sndHead].to = -1;
			sndQueue[sndHead].command = -1;
			sndQueue[sndHead].size = 0;
			sndQueue[sndHead].seqno = -1;
			int i;
			for(i = 0; i < MAX_HOST_NUM; i++){
				sndQueue[sndHead].timestamp[i] = -1;
			}
			return &(sndQueue[sndHead]);
		}
	}else{
		if(recvQueueSize == MAX_QUEUE_SIZE){
			return NULL;
		}else{
			memset(&(recvQueue[recvHead]), 0, sizeof(mimsg_t));
			recvQueue[recvHead].from = -1;
			recvQueue[recvHead].to = -1;
			recvQueue[recvHead].command = -1;
			recvQueue[recvHead].size = 0;
			recvQueue[recvHead].seqno = -1;
			int i;
			for(i = 0; i < MAX_HOST_NUM; i++){
				recvQueue[recvHead].timestamp[i] = -1;
			}
			return &(recvQueue[recvHead]);
		}
	}	
}


/** 
* according to the paramter type, return the pointer to the head of a queue
* parameters
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


/**
* dispatch msg to corresponding handler, according to its command num
* parameters
*	msg : msg to be handled, not NULL
**/
void dispatchMsg(mimsg_t *msg){
	if(msg == NULL || msg->command == -1){
		return;
	}
	switch(msg->command){
		case TEST_COMMAND:
//			printf("receive msg TEST_COMMAND\n");
			testCommand(msg);
			break;
		case ACQ_LOCK:
			printf("receive msg ACQ_LOCK\n");
			handleAcquireMsg(msg);
			break;
		case RLS_LOCK:
//			printf("receive msg RLS_LOCK\n");
			handleReleaseMsg(msg);
			break;
		case GRANT_LOCK:
			printf("receive msg GRANT_LOCK\n");
			handleGrantMsg(msg);
			break;
		case ENTER_BARRIER:
//			printf("receive msg ENTER_BARRIER\n");
			handleEnterBarrierMsg(msg);
			break;
		case EXIT_BARRIER:
//			printf("receive msg EXIT_BARRIER\n");
			handleExitBarrierMsg(msg);
			break;
		case GRANT_WN_I:
//			printf("receive msg GRANT_WN_I\n");
			handleGrantWNIMsg(msg);
			break;
		case GRANT_DIFF:
//			printf("receive msg GRANT_DIFF\n");
			handleGrantDiffMsg(msg);
			break;
		case GRANT_PAGE:
//			printf("receive msg GRANT_PAGE\n");
			handleGrantPageMsg(msg);
			break;
		case FETCH_PAGE:
			printf("receive msg FETCH_PAGE\n");
			handleFetchPageMsg(msg); 
			break;
		case FETCH_WN_I:
//			printf("receive msg FETCH_WN_I\n");
			handleFetchWNIMsg(msg);
			break;
		case FETCH_DIFF:
//			printf("receive msg FETCH_DIFF\n");
			handleFetchDiffMsg(msg);
			break;
		case GRANT_ENTER_BARRIER_INFO:
//			printf("receive msg GRANT_ENTER_BARRIER_INFO\n");
			handleGrantEnterBarrierMsg(msg);
			break;
		case GRANT_EXIT_BARRIER_INFO:
//			printf("receive msg GRANT_EXIT_BARRIER_INFO\n");
			handleGrantExitBarrierMsg(msg);
			break;
		default: 
			fprintf(stderr, "command %d has no handler\n", msg->command);
			break;
	}
}


/**
* append msg to sndQueue, and send all messages out
* parameters
*	msg : msg to be sent
* return value
*	 0 --- success
*	-1 --- parameters error
**/
int sendMsg(mimsg_t *msg){
	if(msg == NULL || msg->from != myhostid || msg->to == -1 || msg->command == -1){
		return -1;
	}
	msg->seqno = datamanager.snd_seqs[msg->to];
	msgEnqueue(0);
	
	//printf("before empty sndQueue\n");
	while(sndQueueSize > 0){
		mimsg_t *m = queueTop(0);
		int to = m->to;
		int from = m->from;
		struct sockaddr_in dest;
		dest.sin_family = AF_INET;
		inet_pton(AF_INET, hosts[to].address, &(dest.sin_addr.s_addr));
		dest.sin_port = dataPorts[to][from];

		int retryNum = 0;
		int success = 0;
		while((retryNum < MAX_RETRY_NUM) && (success != 1)){

			printf("seqno %d : send msg from %d to %d, dest ip = %s, dest port num = %d, command = %d\n", m->seqno, from, to, hosts[to].address, dest.sin_port, msg->command);

			printf("before sendto\n");
			int size = sendto(datamanager.snd_fds[to], m, m->size + MSG_HEAD_SIZE, 0, (struct sockaddr *)&dest, sizeof(dest));
			printf("after sendto\n");

			if(size == -1){
				printf("error occur when sending data\n");
			}

			unsigned long mytimeout = 0;
			if (hostnum <= 8){
         			mytimeout = TIMEOUT - myhostid * 100;
			} else{
         			mytimeout = TIMEOUT - myhostid * 50;
      			}
			unsigned long start = current_time();
      		unsigned long end = start + mytimeout;
			printf("before try\n");
			while((current_time() < end) && (success != 1)){
				fd_set set;
				FD_ZERO(&set);
				int fd = ackmanager.recv_fds[to]; 
				FD_SET(fd, &set);
				struct timeval polltime;
				polltime.tv_sec = 0;
				polltime.tv_usec = 0;
				int num = select(fd+1, &set, NULL, NULL, &polltime);
				if(num == -1){
					printf("err2: %s\n", strerror(errno));
				}
				if(num > 0){
					int seqno = 0;	
					printf("before select\n");
					int size = recvfrom(fd, &seqno, 4, 0, NULL, NULL);
					if(seqno == m->seqno){
						(datamanager.snd_seqs[msg->to])++;
						success = 1;
						printf("seqno %d received ack!\n", seqno);
					}	
				}
			}
			printf("after try\n");
			retryNum++;
		}
		if(success != 1){
			fprintf(stderr, "msg from %d to %d does not receive ack %d\n", m->from, m->to, m->seqno);
		}
		msgDequeue(0);
	}
	//printf("after empty sndQueue\n");
	return 0;
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
