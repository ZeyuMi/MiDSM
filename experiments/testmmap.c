#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>

int main(){
	int fd = open("file", O_RDWR);
	void *a = (int *)mmap((void *)0x60000000, 4096, PROT_READ, MAP_PRIVATE | MAP_FIXED, fd, 0);	
	printf("%p\n", a);
}
