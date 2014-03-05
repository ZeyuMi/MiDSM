#include "../src/net.h"
#include "../src/syn.h"
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>


int myhostid;
int hostnum;
host_t hosts[MAX_HOST_NUM];

int main(){
	myhostid = 0;
	strcpy(hosts[0].address, "192.168.48.42");
	strcpy(hosts[0].username, "yating");
	strcpy(hosts[1].address, "192.168.48.40");
	strcpy(hosts[1].username, "zeyu");
	hostnum = 2;


	initnet();
	initsyn();
	
	printf("enter barrier\n");
	mi_barrier();
	printf("exit barrier\n");

	int i, j, result;
	result = 0;
	for(i = 0; i < 10000; i++){
		printf("grasp lock\n");
		mi_lock(0);
		printf("grasp lock successfully\n");
		for(j = 0; j < 10000; j++){
			result++;
		}
		printf("free lock\n");	
		mi_unlock(0);
		printf("free lock successfully\n");	
	}
	
	printf("enter barrier\n");
	mi_barrier();
	printf("exit barrier\n");
	printf("result = %d\n",result);
}
