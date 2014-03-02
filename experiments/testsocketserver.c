#include<stdio.h>
#include<sys/socket.h>
#include<sys/signal.h>
#include<fcntl.h>
#include<netdb.h>
void sigio_handler(int sigio, siginfo_t *info, void *context){


	printf("entering into sigio_handler\n");
	
}


int main(){
	struct sigaction act;
	act.sa_handler = (void (*)(int))sigio_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGIO, &act, NULL);

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in server, client;
	server.sin_family = AF_INET;
	server.sin_port = 12345;
//	inet_pton(AF_INET, &INADDR_ANY, &(server.sin_addr.s_addr));
	server.sin_addr.s_addr = INADDR_ANY;
	bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
	fcntl(fd, F_SETOWN, getpid());
	fcntl(fd, F_SETFL, O_ASYNC);

	while(1)
		;
}
