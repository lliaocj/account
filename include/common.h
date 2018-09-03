#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define LOG_TAG ""

#define NOTE_LOGE(fmt,ptr...)  printf("\033[31m"LOG_TAG":%s[%d]"fmt"\033[0m\n",__FUNCTION__,__LINE__, ## ptr)
#define NOTE_LOGD(fmt,ptr...)  printf("\033[37m"LOG_TAG":%s[%d]"fmt"\033[0m\n",__FUNCTION__,__LINE__, ## ptr)
#define __PACKED__        __attribute__ ((__packed__))

typedef enum
{
	NOTE_ADD_USER,
	NOTE_DEL_USER,
	NOTE_ADD_BILL,
	NOTE_SYNC_BILL,
	NOTE_CONNECT,
	NOTE_ADD_BANK,
	NOTE_GET_BANK,
	NOTE_DEL_BANK,
	NOTE_DEL_BILL
}NOTE_CMD;

struct __NOTE_MSG_HEARD
{
	char cmd;
	int  s32size;
	char name[12];
}__PACKED__;
typedef struct __NOTE_MSG_HEARD NOTE_MSG_HEARD;

struct __NOTE_MSG_CONNECT
{
	char mID[6];
	char mTable[12];
	char name[12];
	char upDate[32];
}__PACKED__;

typedef struct __NOTE_MSG_CONNECT NOTE_MSG_CONNECT;

struct __NOTE_MSG_PACKET
{
	char money[8];
	char name[12];
	char group[4];
	char date[32];
	char note[256];
}__PACKED__;

typedef struct __NOTE_MSG_PACKET NOTE_MSG_PACKET;

#endif
