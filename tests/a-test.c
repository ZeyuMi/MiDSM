#include <stdio.h>
#include "a.h"
#include "minunit.h"

int tests_run = 0;

static char *test_sum(){
	mu_assert("sum fail", sum(2,3) == 5);
	return 0;
}

static char *all_tests(){
	mu_run_test(test_sum);
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
