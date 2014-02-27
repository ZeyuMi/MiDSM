#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>

int main(){
	int fd = open("/dev/zero", O_RDWR, 0);
	printf("%d\n", fd);
	int *a = (int *)mmap((void *)0x60000000, 4096, PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_FIXED, fd, 0);	
	*a = 3;
	printf("%d\n", *a);
	*(a+1023) = 3;
}
