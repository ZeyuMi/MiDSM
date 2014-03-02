#include "../src/net.h"
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>


extern int myhostid;
extern host_t hosts[MAX_HOST_NUM];

void sigio_handler1(int sigio, siginfo_t *info, void *context){


	printf("entering into sigio_handler\n");
	
}


void notify(){
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in to;
	to.sin_family = AF_INET;
	to.sin_port = 33233;
	inet_pton(AF_INET, "192.168.48.42", &(to.sin_addr.s_addr));
	char *meg = "hello, world";
	sendto(fd, meg, strlen(meg)+1, 0, &to, sizeof(to));
	close(fd);
	
}
int main(){
	myhostid = 1;
	strcpy(hosts[0].address, "192.168.48.42");
	strcpy(hosts[0].username, "yating");
	strcpy(hosts[1].address, "192.168.48.40");
	strcpy(hosts[1].username, "zeyu");

	notify();

	struct sigaction act;
	act.sa_handler = (void (*)(int))sigio_handler1;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGIO, &act, NULL);

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in server, client;
	server.sin_family = AF_INET;
	server.sin_port = 12386;
//	inet_pton(AF_INET, &INADDR_ANY, &(server.sin_addr.s_addr));
	server.sin_addr.s_addr = INADDR_ANY;
	bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
	fcntl(fd, F_SETOWN, getpid());
	fcntl(fd, F_SETFL, O_ASYNC);


	while(1)
		;
}
