#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include "init.h"
#include "net.h"
#include "syn.h"

host_t hosts[MAX_HOST_NUM];
int hostnum = 0;
int myhostid = 0;


/**
* initialization of whole program
**/
void mi_init(int argc, char **argv){
	if(argc <= 0 || argv == NULL || *argv == NULL){
		printf("argument error\n");
		exit(1);
	}
	initLocalEnv();
	if(myhostid == 0){
		printf("node 0 is starting other node...\n");
		startNodePrograms(argc, argv);
	}
}


/**
* start program in other nodes
**/
void startNodePrograms(int argc, char **argv){
	int i;
	char wd[WORDSIZE];
	char address[WORDSIZE];
	char cmd[WORDSIZE*2];
	for(i = 1; i < hostnum; i++){
		memset(wd, 0, WORDSIZE);
		memset(address, 0, WORDSIZE);
		memset(cmd, 0, WORDSIZE*2);
			
		strcat(wd, "/home/");
		strcat(wd, hosts[i].username);
		strcat(wd, "/Desktop/MiDSM/tests/");

		strcat(address, hosts[i].username);
		strcat(address, "@");
		strcat(address, hosts[i].address);

		sprintf(cmd, "ssh %s \"cd %s; ", address, wd);	

		strcat(cmd, *argv);
		strcat(cmd, " &\"");
		system(cmd);
	}
}


/**
* initialize hosts array, set myhostid value and call other module initialization procedure
**/
void initLocalEnv(){
	readHosts(".mihosts");
	int id = getuid();
	struct passwd *user = getpwuid(id);
	myhostid = findHostIdByName(user->pw_name);
	initnet();
	initsyn();
} 


/**
* read hosts from .mihosts file
* parameters
*	filename : the name of file which contains information about ip and username
**/
void readHosts(char *filename){
	FILE *hostfile = fopen(filename, "r");
	if(hostfile == NULL){
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
		readAddrFromStr(line, hosts[hosti].address, WORDSIZE);						
		readNameFromStr(line, hosts[hosti].username, WORDSIZE);
		hostnum++;
		hosti++;
	}
}

/**
* find the host index of this node
* after this procedure, global variable myhostid will be given a value
* return value
*	>= 0 --- id 
*	  -1 --- error
**/
int findHostIdByName(char *name){
	if(name == NULL){
		return -1;
	}
	int i = 0;
	while(i < hostnum){
		if(strcmp(hosts[i].username, name) == 0){
			return i;
		}
		i++;
	}
	return -1;
}


/**
* find ip address from str, store it into addr
* parameters
*	addr is the returned ip
* 	addrsize is the size of addr buffer
* return value
*	 0 --- SUCCESS
*	-1 --- ERROR
**/
int readAddrFromStr(const char *str, char *addr, int addrsize){
	return readSegFromStr(str, 0, addr, addrsize);
}


/**
* find username from str, store it into username
* parameters
*	username is the returned username
* 	usernamesize is the size of username buffer
* return value
*	 0 --- SUCCESS
*	-1 --- ERROR
**/
int readNameFromStr(const char *str, char *username, int usernamesize){
	return readSegFromStr(str, 1, username, usernamesize);
}


/**
* str consists of different segments, which are splitted by ' '. This procedure extract the specified segment.
* parameters
*	index begins from 0
* 	content is the returned segment
* 	contentsize is the size of content
* return value
*	 0 --- SUCCESS 
*	-1 --- ERROR
**/
int readSegFromStr(const char *str, int index, char *content, int contentsize){
	if(str == NULL || index < 0 || content == NULL || contentsize <= 0){
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
