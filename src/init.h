#define WORDSIZE 80
#define MAX_HOST_NUM 20

typedef struct{
	char address[WORDSIZE];
	char username[WORDSIZE];
	} host_t;

void mi_init();
