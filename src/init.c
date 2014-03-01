#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include "init.h"

host_t hosts[MAX_HOST_NUM];
int hostnum = 0;
int myhostid = 0;

void readHosts();
void findMyHostId();
int readAddrFromStr(const char *str, int strlen, char *addr, int addrsize);
int readNameFromStr(const char *str, int strlen, char *username, int usernamesize);
int readSegFromStr(const char *str, int strlen, int index, char *content, int contentsize);


void mi_init(){
	readHosts();
	findMyHostId();
}


/**
* read hosts from .mihosts file
**/
void readHosts(){
	FILE *hostfile = fopen(".mihosts", "r");
	if(hostfile == 0){
		fprintf(stderr, ".mihosts cannot be opend\n");
		exit(1);
	}
	char *line = NULL;
	size_t llen = 0;
	int read = 0;
	int hosti = 0;
	while((read = getline(&line, &llen, hostfile)) != -1){
		int addrlen = 0;
		int namelen = 0;
		readAddrFromStr(line, llen, hosts[hosti].address, WORDSIZE);						
		readNameFromStr(line, llen, hosts[hosti].username, WORDSIZE);
		hostnum++;
		hosti++;
	}
}

/**
* find the host index of this node
* after this procedure, global variable myhostid will be given a value
**/
void findMyHostId(){
	int id = getuid();
	struct passwd *user = getpwuid(id);
	int i = 0;
	while(i < hostnum){
		if(strcmp(hosts[i].username, user->pw_name) == 0){
			myhostid = i;
			return;
		}
	}
}


/**
* find ip address from str, store it into addr
* parameters
*	addr is the returned ip
* 	addrsize is the size of addr buffer
* return value
*	-1 --- ERROR
*	 0 --- SUCCESS
**/
int readAddrFromStr(const char *str, int strlen, char *addr, int addrsize){
	return readSegFromStr(str, strlen, 0, addr, addrsize);
}


/**
* find username from str, store it into username
* parameters
*	username is the returned username
* 	usernamesize is the size of username buffer
* return value
*	-1 --- ERROR
*	 0 --- SUCCESS
**/
int readNameFromStr(const char *str, int strlen, char *username, int usernamesize){
	return readSegFromStr(str, strlen, 1, username, usernamesize);
}

/**
* str consists of different segments, which are splitted by ' '. This procedure extract the specified segment.
* parameters
*	index begins from 0
* 	content is the returned segment
* 	contentsize is the size of content
* return value
*	-1 --- ERROR
*	 0 --- SUCCESS 
**/
int readSegFromStr(const char *str, int strlen, int index, char *content, int contentsize){
	if(str == NULL || index < 0 || content == NULL || contentsize == 0){
		return -1;
	}
	int ch = 0;
	const char *head = str;
	const char *tail = str;
	int len = 0;
	while(((ch = *tail) == ' ' || ch == '\t') && ch != '\0' && ch != '\n')
		tail++;
	while(index >= 0){
		head = tail;
		len = 0;
		while((ch = *tail) != ' ' && ch != '\t' && ch != '\0' && ch != '\n' && len < contentsize-1){
			tail++;
			len++;
		}
		index--;
		//segment is longer than content buffer
		if(ch != ' ' && ch != '\t' && ch != '\0'&& ch != '\n' && len == contentsize-1)
			return -1; 

		while(((ch = *tail) == ' ' || ch == '\t') && ch != '\0' && ch != '\n')
			tail++;
		if(ch == '\0' || ch == '\n')
			break;
	}
	if((ch == '\0' || ch == '\n')&& index > 0)
		return -1;
	else{
		strncpy(content, head, len);
		*(content+len) = '\0';
		return 0;
	}
}
