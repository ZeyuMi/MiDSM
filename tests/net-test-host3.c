#include "../src/net.h"
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>


extern int myhostid;
extern host_t hosts[MAX_HOST_NUM];

void notify(){
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in to;
	to.sin_family = AF_INET;
	to.sin_port = 33233;
	inet_pton(AF_INET, "192.168.48.42", &(to.sin_addr.s_addr));
	char *meg = "hello, world";
	sendto(fd, meg, strlen(meg)+1, 0, (struct sockaddr *)&to, sizeof(to));
	close(fd);
	
}
int main(){
	myhostid = 1;
	strcpy(hosts[0].address, "192.168.48.42");
	strcpy(hosts[0].username, "yating");
	strcpy(hosts[1].address, "192.168.48.40");
	strcpy(hosts[1].username, "zeyu");

	notify();

	initnet();
	mimsg_t *m1 = newMsg();
	m1->from = 1;
	m1->to = 0;
	m1->command = TEST_COMMAND;
	char *s1 = "test from 1!";
	apendMsgData(m1, s1, strlen(s1)+1);
	sendMsg(m1);
	freeMsg(m1);

	m1 = newMsg();
	m1->from = 1;
	m1->to = 0;
	m1->command = TEST_COMMAND;
	apendMsgData(m1, s1, strlen(s1)+1);
	sendMsg(m1);
	freeMsg(m1);


	while(1)
		;

}
