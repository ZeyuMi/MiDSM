#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<netdb.h>

int main(){
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in to;
	to.sin_family = AF_INET;
	to.sin_port = 12345;
	inet_pton(AF_INET, "192.168.48.42", &(to.sin_addr.s_addr));
	char *meg = "hello, world";
	sendto(fd, meg, strlen(meg)+1, 0, &to, sizeof(to));
	close(fd);
	
}
