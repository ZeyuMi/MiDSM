#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/mem.h"
#include "../src/init.h"
#include "minunit.h"
int tests_run = 0;

int myhostid;
int hostnum;
extern page_t pageArray[MAX_PAGE_NUM];

static char *test_isAfterInterval(){
	int a[MAX_HOST_NUM];
	int b[MAX_HOST_NUM];
	memset(a, 0, MAX_HOST_NUM);
	memset(b, 0, MAX_HOST_NUM);

	hostnum = 4;
	
	a[0] = 1;
	b[0] = 2;
	mu_assert("mem1", isAfterInterval(a, b) == 0);

	a[0] = 1;
	b[0] = 1;
	mu_assert("mem2", isAfterInterval(a, b) == 0);

	a[1] = 2;
	b[1] = 1;
	mu_assert("mem3", isAfterInterval(a, b) == 1);

	a[2] = 2;
	b[2] = 0;
	mu_assert("mem4", isAfterInterval(a, b) == 1);

	a[4] = 2;
	b[4] = 3;
	mu_assert("mem5", isAfterInterval(a, b) == 1);


	mu_assert("mem6", isAfterInterval(NULL, b) == -1);
	mu_assert("mem7", isAfterInterval(a, NULL) == -1);
	mu_assert("mem8", isAfterInterval(NULL, NULL) == -1);
	
	return 0;
}


static char *test_createTwinPage(){
	memset(pageArray, 0, MAX_PAGE_NUM * sizeof(page_t));
	pageArray[0].address = malloc(PAGESIZE);
	pageArray[0].state = RDONLY;		
	pageArray[0].twinPage = NULL;
	memset(pageArray[0].address, 0, PAGESIZE);
	((int *)(pageArray[0].address))[2] = 3;
	((int *)(pageArray[0].address))[1023] = 1023;
	mu_assert("mem9", createTwinPage(0) == 0);
	mu_assert("mem10", ((int *)pageArray[0].twinPage)[2] == 3);
	mu_assert("mem11", ((int *)pageArray[0].twinPage)[1023] == 1023);
	mu_assert("mem12", createTwinPage(0) == -1);
	mu_assert("mem13", createTwinPage(1) == -1);
	mu_assert("mem14", createTwinPage(-1) == -1);
	mu_assert("mem15", createTwinPage(MAX_PAGE_NUM) == -1);
	pageArray[1].address = malloc(PAGESIZE);
	pageArray[1].state = MISS;		
	pageArray[1].twinPage = NULL;
	mu_assert("mem16", createTwinPage(1) == -1);
	free(pageArray[0].address);
	free(pageArray[0].twinPage);
	free(pageArray[1].address);

	return 0;
}


static char *test_freeTwinPage(){
	memset(pageArray, 0, MAX_PAGE_NUM * sizeof(page_t));
	pageArray[0].address = malloc(PAGESIZE);
	pageArray[0].state = RDONLY;		
	pageArray[0].twinPage = NULL;
	memset(pageArray[0].address, 0, PAGESIZE);
	((int *)(pageArray[0].address))[2] = 3;
	((int *)(pageArray[0].address))[1023] = 1023;
	mu_assert("mem17", createTwinPage(0) == 0);
	mu_assert("mem18", ((int *)pageArray[0].twinPage)[2] == 3);
	mu_assert("mem19", ((int *)pageArray[0].twinPage)[1023] == 1023);
	mu_assert("mem20", freeTwinPage(0) == 0);
	mu_assert("mem21", pageArray[0].twinPage == NULL);
	
	mu_assert("mem22", freeTwinPage(-1) == -1);
	mu_assert("mem23", freeTwinPage(MAX_PAGE_NUM) == -1);
	mu_assert("mem24", freeTwinPage(0) == -1);
	mu_assert("mem13", freeTwinPage(3) == -1);

	free(pageArray[0].address);
	return 0;
}


static char *all_tests(){
	mu_run_test(test_isAfterInterval);
	mu_run_test(test_createTwinPage);
	mu_run_test(test_freeTwinPage);
	return 0;
}


int main(){
	char *result = all_tests();
	if(result != 0){
		printf("%s\n", result);
	}else{
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run : %d\n", tests_run);
	return result != 0;
}

