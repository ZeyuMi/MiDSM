#define WORDSIZE 80
#define MAXHOSTNUM 20

typedef struct{
	char address[WORDSIZE];
	char username[WORDSIZE];
	} host_t;

void mi_init();
