#include "net.h"
#include "util.h"

//extern int myhostid;
//extern host_t hosts[MAX_HOST_NUM];
int myhostid;
host_t hosts[MAX_HOST_NUM];

mimsg_t sndQueue[MAX_QUEUE_SIZE];
mimsg_t recvQueue[MAX_QUEUE_SIZE];
int dataPorts[MAX_HOST_NUM][MAX_HOST_NUM];
int ackPorts[MAX_HOST_NUM][MAX_HOST_NUM];
int sndHead, sndTail, sndQueueSize, recvHead, recvTail, recvQueueSize;
netmanager datamanager;
netmanager ackmanager;

void testCommand(mimsg_t *msg){
	printf("msg from %d to %d with command %d received!\n", msg->from, msg->to, msg->command);
}

void sigio_handler(int sigio, siginfo_t *info, void *context){


	printf("entering into sigio_handler\n");


	fd_set readset = datamanager.recv_fdset;
	struct timeval polltime;
	polltime.tv_sec = 0;
	polltime.tv_usec = 0;
	int num = select(datamanager.recv_maxfd, &readset, NULL, NULL, &polltime);
	if(num > 0){
		int i;
		for(i = 0; i < MAX_HOST_NUM; i++){
			if(i != myhostid){
				int fd = datamanager.recv_fds[i];
				if(FD_ISSET(fd, &readset)){
					mimsg_t *msg = newMsg();
					struct sockaddr_in addr;	
					int s = sizeof(addr);
					int size = recvfrom(fd, msg, sizeof(mimsg_t), 0, &addr, &s);
					if(size > 0){
						struct sockaddr_in dest;
						dest.sin_family = AF_INET;
						inet_pton(AF_INET, hosts[i].address, &(dest.sin_addr.s_addr));
						dest.sin_port = ackPorts[msg->from][myhostid];
						sendto(ackmanager.snd_fds[i], &(msg->seqno), 4, 0, &dest, sizeof(dest));
					}
					msgEnqueue(1, msg);	
					freeMsg(msg);
				}
			}
		}
		readset = datamanager.recv_fdset;
		polltime.tv_sec = 0;
		polltime.tv_usec = 0;
		num = select(datamanager.recv_maxfd, &readset, NULL, NULL, &polltime);
	}
	
	while(recvQueueSize > 0){
		mimsg_t *msg = queueTop(1);
		dispatchMsg(msg);
		msgDequeue(1);
	}
	
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


/**
* dispatch msg to corresponding handler, according to its command num
* parameters
*	msg : msg to be handled, not NULL
**/
void dispatchMsg(mimsg_t *msg){
	if(msg == NULL || msg->command == -1){
		return NULL;
	}
	switch(msg->command){
		case TEST_COMMAND:
			testCommand(msg);
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
	msgEnqueue(0, msg);
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

			printf("send msg from %d to %d, dest ip = %s, dest port num = %d\n", from, to, hosts[to].address, dest.sin_port);


			int size = sendto(datamanager.snd_fds[to], m, m->size + MSG_HEAD_SIZE, 0, &dest, sizeof(dest));

			if(size == -1){
				printf("error occur when sending data\n");
			}
			fd_set set;
			FD_ZERO(&set);
			int fd = ackmanager.recv_fds[to]; 
			FD_SET(fd, &set);
			struct timeval polltime;
			polltime.tv_sec = 1;
			int num = select(fd+1, &set, NULL, NULL, &polltime);
			if(num > 0){
				mimsg_t *ackmsg = newMsg();
				int seqno = 0;
				int size = recvfrom(fd, &seqno, 4, 0, NULL, NULL);
				if(seqno == m->seqno){
					success = 1;
				}	
			}
			retryNum++;
		}
		if(success != 1){
			fprintf(stderr, "msg from %d to %d does not receive ack %d\n", m->from, m->to, m->seqno);
		}
		msgDequeue(0);
	}
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


