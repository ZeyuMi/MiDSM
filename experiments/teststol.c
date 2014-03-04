#include <stdlib.h>
#include <stdio.h>

int main(){
	char buffer[20];
	int i =0;
	for(; i < 20; i++){
		buffer[i] = 0;
	}
	sprintf(buffer, "%d", 3);
	for(i = 0; i < 20; i++){
		printf("%d\n", buffer[i]);
	}
}
