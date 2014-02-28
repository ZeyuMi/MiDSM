#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "mem.h"


mipage_t pages[MAXPAGENUM];
int pagenum;
long mapfd;
long globalAddress;

void handler(int signo, siginfo_t *info, void *context){
	address_t faultaddress = info->si_addr;
	address_t addr = ((long)faultaddress % PAGESIZE == 0) ? faultaddress : ((long)faultaddress / PAGESIZE * PAGESIZE) ;
	/*check whether addr is in valid range*/
	address_t maxaddr = pages[pagenum-1].addr + PAGESIZE;
	if(addr < STARTADDRESS || addr >= maxaddr){
		printf("invalid address\n");
		exit(1);
	}
	int pageindex = ((long)addr - STARTADDRESS) / PAGESIZE;
	if(pages[pageindex].state == RDONLY){
		pages[pageindex].state = WRITE;
		if(mprotect(pages[pageindex].addr, PAGESIZE, PROT_READ | PROT_WRITE) == -1)
			printf("error\n");
		/*create twin page*/
		printf("read to write\n");


	}else if(pages[pageindex].state == MISS){

	}	
}


void *mi_alloc(int size){
	int rsize = (size % PAGESIZE == 0) ? size : (size / PAGESIZE + 1) * PAGESIZE;
	int allocesize = rsize;
	if(rsize + globalAddress > MAXMEMSIZE)
		return NULL;
	while(allocesize > 0){
		void *a = mmap((void *)(STARTADDRESS + globalAddress), 4096, PROT_READ , MAP_PRIVATE | MAP_FIXED, mapfd, 0);	
		printf("%p\n", a);
		pages[pagenum].addr = (address_t)STARTADDRESS + globalAddress;
		pages[pagenum].state = RDONLY;
		pagenum++;
		allocesize -= PAGESIZE;
	}
	globalAddress += rsize;
	return (void *)(STARTADDRESS + globalAddress - rsize);
}


/**
* initialization work of memory management before the system is normally used 
**/
void init_mem(){
/*******************install segv signal handler*************/
	struct sigaction act;
	act.sa_handler = (void (*)(int))handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &act, NULL);
/*******************initialize pages************************/
	int i;
	for(i = 0; i < MAXPAGENUM; i++){
		pages[i].addr = 0;
		pages[i].state = 0;	
	}
/*******************prepare mapped file*********************/	
	mapfd = open("/dev/zero", O_RDWR, 0);
/*******************initialize global variables*********************/	
	globalAddress = 0;
	pagenum = 0;
}


int main(){
	init_mem();
	int *a = (int *)mi_alloc(4096);
	*a = 1;
	printf("%d\n", *a);
	*(a+1023) = 2;
	printf("%d\n", *(a+1023));
	*(a+1024) = 3;
	printf("%d\n", *(a+1024));
}
