#include<stdio.h>
#include<signal.h>

void handler(int signo, siginfo_t *info, void *context){
	printf("address is %p\n", info->si_addr);
	exit(0);
}

int main(){
	struct sigaction act;
	act.sa_handler = (void (*)(int))handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &act, NULL);
	int *a = 1;
	*a = 2;
	printf("end of main\n");
}
