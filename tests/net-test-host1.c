#include "../src/net.h"
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>


int myhostid;
int hostnum;
host_t hosts[MAX_HOST_NUM];
void wait(){
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in server, client;
	server.sin_family = AF_INET;
	server.sin_port = 33233;
//	inet_pton(AF_INET, &INADDR_ANY, &(server.sin_addr.s_addr));
	server.sin_addr.s_addr = INADDR_ANY;
	bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
	char buf[128];
	int size = recvfrom(fd, buf, 128, 0, NULL, NULL);
	printf("%s\n", buf);		
}
int main(){
	myhostid = 0;
	strcpy(hosts[0].address, "192.168.48.42");
	strcpy(hosts[0].username, "yating");
	strcpy(hosts[1].address, "192.168.48.40");
	strcpy(hosts[1].username, "zeyu");
	hostnum = 2;
	wait();

	initnet();

	mimsg_t *m1 = newMsg();
	m1->from = 0;
	m1->to = 1;
	m1->command = TEST_COMMAND;
	char *s1 = "test from 0!";
	apendMsgData(m1, s1, strlen(s1)+1);
	sendMsg(m1);
	freeMsg(m1);

	m1 = newMsg();
	m1->from = 0;
	m1->to = 1;
	m1->command = TEST_COMMAND;
	apendMsgData(m1, s1, strlen(s1)+1);
	sendMsg(m1);
	freeMsg(m1);

	
	while(1)
		;
}
