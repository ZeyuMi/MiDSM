#include "../src/net.h"

mimsg_t msg;
int parametertype;
int sendMsgCalled = 0;
int nextFreeMsgInQueueCalled = 0;

mimsg_t *nextFreeMsgInQueue(int type){
	parametertype = type;
	nextFreeMsgInQueueCalled = 1;
	return &msg;
}


int sendMsg(mimsg_t *msg){
	sendMsgCalled = 1;	
}

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

void disableSigio(){

}

void enableSigio(){

}
