#include <signal.h>
#include <fcntl.h>
#include "mem.h"


mipage_t pages[MAXPAGENUM];
long mapfd;

void handler(int signo, siginfo_t *info, void *context){
		
}


void *mi_alloc(int size){
	

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
	for(int i = 0; i < MAXPAGENUM; i++){
		pages[i].addr = 0;
		pages[i].state = 0;	
	}
/*******************prepare mapped file*********************/	
	mapfd = open("/dev/zero", O_RDWR);
}
