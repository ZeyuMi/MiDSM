#include "../src/net.h"
#include "../src/syn.h"
#include "../src/init.h"
#include "../src/mem.h"
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>


int main(int argc, char **argv){
	mi_init(argc, argv);
	
	printf("enter barrier\n");
	mi_barrier();
	printf("exit barrier\n");

	int i, j;
	int *result = (int *)mi_alloc(sizeof(int));
	printf("enter barrier\n");
	mi_barrier();
	printf("exit barrier\n");

	for(i = 0; i < 50; i++){	
		printf("before lock\n");
		mi_lock(0);
		printf("after lock\n");
		*result = *result + 1;
		printf("before unlock\n");
		mi_unlock(0);
		printf("after unlock\n");
	}
	
	printf("enter barrier\n");
	mi_barrier();
	printf("exit barrier\n");
	printf("result = %d\n", *result);
	showDataStructures();
}
