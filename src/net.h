#ifndef MINET_H
#define MINET_H

#include "init.h"
#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#

#define MSG_HEAD_SIZE ((MAX_HOST_NUM+3)*4)
#define MAX_MSG_SIZE 40960
#define MAX_QUEUE_SIZE MAX_HOST_NUM
#define BASEPORT 12345
#define MAX_RETRY_NUM 64

#define TEST_COMMAND 0
typedef struct {
		int from;
		int to;
		int command;
		int size;
		int seqno;
		int timestamp[MAX_HOST_NUM];	
		char data[MAX_MSG_SIZE-MSG_HEAD_SIZE];
	} mimsg_t;
typedef struct {
		int snd_fds[MAX_HOST_NUM];
		int snd_seqs[MAX_HOST_NUM];
		int recv_fds[MAX_HOST_NUM];
		fd_set recv_fdset;
		int recv_maxfd;
	} netmanager;

void initnet();
int createSocket(short int port, int isRecv, int bufSize);
int msgEnqueue(int type, mimsg_t *msg);
mimsg_t *queueTop(int type);
int msgDequeue(int type);
void dispatchMsg(mimsg_t *msg);
int sendMsg(mimsg_t *msg);
int apendMsgData(mimsg_t *msg, char *data, int len);
mimsg_t *newMsg();
int freeMsg(mimsg_t *msg);
#endif
