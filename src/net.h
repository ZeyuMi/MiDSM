#ifndef MINET_H
#define MINET_H

#include "init.h"
#define MAX_HEAD_SIZE ((MAX_HOST_NUM+3)*4)
#define MAX_MSG_SIZE 40960
#define MAX_QUEUE_SIZE MAX_HOST_NUM

typedef struct {
		int from;
		int to;
		int command;
		int timestamp[MAX_HOST_NUM];	
		char data[MAX_MSG_SIZE-MAX_HEAD_SIZE]
	} mimsg_t;

void initnet();
int sendMsg(mimsg_t *msg);
int apendMsgData(mimsg_t *msg, char *data, int len);
mimsg_t *newMsg();
int freeMsg(mimsg_t *msg);
#endif
