#define WORDSIZE 80
#define MAXHOSTNUM 20

typedef struct{
	char address[80];
	char username[80];
	int addrlen;
	int usernamelen;
	} host_t;

void mi_init();
