#include<stdio.h>
#include<sys/socket.h>
#include<netdb.h>

int main(){
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server, client;
	server.sin_family = AF_INET;
	server.sin_port = 12345;
//	inet_pton(AF_INET, &INADDR_ANY, &(server.sin_addr.s_addr));
	server.sin_addr.s_addr = INADDR_ANY;
	bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
	listen(fd, 10);
	int addrsize = sizeof(struct sockaddr_in);	
	int clientFd = accept(fd, (struct sockaddr *)&client, &addrsize);
	char buf[128];
	int size = recv(clientFd, buf, 128, 0);
	printf("%s\n", buf);		
}
