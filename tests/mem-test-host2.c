#include "../src/net.h"
#include "../src/syn.h"
#include "../src/init.h"
#include "../src/mem.h"
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>


int main(int argc, char **argv){
	printf("hello,world from host2\n");
	mi_init(argc, argv);
	
	printf("enter barrier\n");
	mi_barrier();
	printf("exit barrier\n");

	int i, j;
//	int *result = (int *)mi_alloc(sizeof(int));
	printf("enter barrier\n");
	mi_barrier();
	printf("exit barrier\n");

	for(i = 0; i < 10; i++){
		mi_lock(0);
//		*result = *result + 1;
		mi_unlock(0);
	}
	
	
	printf("enter barrier\n");
	mi_barrier();
	printf("exit barrier\n");
//	printf("result = %d\n", *result);
}
