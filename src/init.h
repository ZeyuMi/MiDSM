#ifndef MIINIT_H
#define MIINIT_H

#define WORDSIZE 80
#define MAX_HOST_NUM 20


typedef struct{
	char address[WORDSIZE];
	char username[WORDSIZE];
	} host_t;

void mi_init(int argc, char **argv);
void startNodePrograms(int argc, char **argv);
void initLocalEnv();
void readHosts(char *filename);
int findHostIdByName(char *name);
int readAddrFromStr(const char *str, char *addr, int addrsize);
int readNameFromStr(const char *str, char *username, int usernamesize);
int readSegFromStr(const char *str, int index, char *content, int contentsize);
#endif
