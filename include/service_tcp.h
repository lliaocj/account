#ifndef _SERVICE_TCP_H_
#define _SERVICE_TCP_H_
#include "common.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdio.h>   
#include <unistd.h>  
#include <stdlib.h>  
#include <errno.h>  
#include <string.h>  
#include <sys/socket.h>  
#include <sys/types.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <sys/select.h>

#define TCP_MAX_CLIENT  10
#define TCP_PORT        1980

typedef struct 
{
	int fd;
	char ip[16];
	int use;
	int index;
}ClientAddr;

typedef struct 
{
	int fd;
	unsigned char *pBuff;
	int u32size;
}SERVICE_MSG;

typedef enum
{
	SERVICE_TCP_CONNECT,
	SERVICE_TCP_DISCONNECT,
	SERVICE_TCP_RECV_MSG,
	SERVICE_TCP_BUFF
}SERVICE_TCP_STATE;

typedef void (*funListen)(SERVICE_TCP_STATE cmd, void *arg);


extern int service_tcp_init();
extern int service_tcp_SendData(int SockFd, unsigned char *pBuff, int u32size);
extern void service_tcp_SetCallback(funListen pFunction);


#endif
