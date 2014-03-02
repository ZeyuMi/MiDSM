#include <stdio.h>
#include <string.h>
#include "../src/init.h"
#include "minunit.h"
int tests_run = 0;

static char *test_readSegFromStr(){
	int bufsize = 80;
	char buf[bufsize];
	char *str = "12 22 33 dff 2ds";
		
	mu_assert("init1", readSegFromStr(NULL, 2, buf, bufsize) == -1);
	mu_assert("init2", readSegFromStr(str, -1, buf, bufsize) == -1);
	mu_assert("init3", readSegFromStr(str, 2, NULL, bufsize) == -1);
	mu_assert("init4", readSegFromStr(str, 2, buf, 0) == -1);
	mu_assert("init5", readSegFromStr(str, 2, buf, bufsize) == 0);
	mu_assert("init6", strcmp(buf, "33") == 0);
	mu_assert("init7", readSegFromStr(str, 10, buf, bufsize) == -1);
	
	return 0;
}


static char *test_findHostIdByName(){
	readHosts("../src/.mihosts");
	mu_assert("init8", findHostIdByName("yating") == 0);
	mu_assert("init9", findHostIdByName("zeyu") == 1);
	mu_assert("init10", findHostIdByName(NULL) == -1);
	mu_assert("init11", findHostIdByName("sss") == -1);
	
	return 0;
}


static char *test_startNodePrograms(){
	char *a = "dsm";	
	startNodePrograms(1,  &a);
	return 0;
}


static char *all_tests(){
	mu_run_test(test_readSegFromStr);
	mu_run_test(test_findHostIdByName);
	mu_run_test(test_startNodePrograms);
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

