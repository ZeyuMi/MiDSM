#include <stdio.h>
#include "init.h"

host_t host[MAXHOSTNUM];
int hostnum = 0;
int myhostid = 0;

void readHosts();
void findMyHostId();
void readAddrFromStr(char *str, int strlen, char **addr, int *addrlen);
void readNameFromStr(char *str, int strlen, char **username, int usernamelen);
int readSegFromStr(const char *str, int strlen, int index, char **content, int *contentlen);


void mi_init(){
	readHosts();
	findMyHostId();
}


/**
*read hosts from .mihosts file
**/
void readHosts(){
	FILE *hostfile = fopen(".mihosts", "r");
	if(hostfile == 0){
		fprintf(STDERR, ".mihosts cannot be opend\n");
		exit(1);
	}
	char *line = NULL;
	size_t llen = 0;
	int read = 0;
	int hosti = 0;
	while((read = getline(&line, &llen, hostfile)) != -1){
		int addrlen = 0;
		int namelen = 0;
		readAddrFromStr(line, llen, &(hosts[hosti].address), &(hosts[hosti].addrlen));						
		readNameFromStr(line, llen, &(hosts[hosti].username), &(hosts[hosti].addrlen));
		hostnum++;
		hosti++;
	}
}


/**
* str consists of different segments, which are splitted by ' '. This procedure extract the specified segment.
* index begins from 0
* content is the returned segment
* contentlen is the length of the returned segment
* return value
*	-1 --- ERROR
*	 0 --- SUCCESS 
**/
int readSegFromStr(const char *str, int strlen, int index, char **content, int *contentlen){
	if(str == NULL || index < 0 || content == NULL || contentlen == NULL){
		return -1;
	}
	int ch = 0;
	char *head = str;
	char *tail = str;
	int len = 0;
	while(((ch = *tail) == ' ' || ch == '\t') && ch != '\0')
		tail++;
	while(index >= 0){
		head = tail;
		len = 0;
		while((ch = *tail) != ' ' || ch != '\t' || ch != '\0'){
			tail++;
			len++;
		}
		index--;
		while(((ch == *tail) == ' ' || ch == '\t') && ch != '\0')
			tail++;
		if(ch == '\0')
			break;
	}
	
	

}


