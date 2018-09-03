#ifndef __SQLITE__H__
#define __SQLITE__H__

#include <sqlite3.h>
#include "common.h"

#if 0
typedef struct 
{
	unsigned char index[16];
	unsigned char name[16];
    unsigned char money[6];
	unsigned char note[128];
}S_DB_INFO;
#endif

#define  S_DB_INFO NOTE_MSG_PACKET


#define DB_FILE_PATH  "/data/note_service.db"
#define DB_PERSON_TABLE "person"
#define DB_BANK_TABLE "bank"

#define DB_CMD_TABLE_IS_EXITS   "select count(*) from sqlite_master where type='table' and name='%s';"
#define DB_CMD_TABLE_CREATE     "create table m%s(id integer primary key, name text, grop integer , mm float,note text);"
#define DB_CMD_TABLE_INSERT     "insert into m%s values(%s,'%s', %s, %s,'%s');"
#define DB_CMD_TABLE_DEL        "delete from m%s where id = %s;"
#define DB_CMD_TABLE_SELECT_ALL "select * from m%s order by id;"

#define DB_CMD_TABLE_USER_CREATE "create table person(name text primary key, date text);"
#define DB_CMD_TABLE_USER_ADD   "insert into person values('%s', '%s');"
#define DB_CMD_TABLE_USER_DEL   "delete from person where name = '%s';"
#define DB_CMD_TABLE_USER_SET_DATE   "update person set update = '%s' where name = '%s';"
#define DB_CMD_TABLE_USER_GET_DATE   "select * from person where name = '%s';"

#define DB_CMD_TABLE_BANK_CREATE "create table %s_%s(id integer, name text, total float, note text);"
#define DB_CMD_TABLE_BANK_ADD    "insert into %s_%s values(%s, '%s', %s, '%s');"
#define DB_CMD_TABLE_BANK_DEL   "delete from %s_%s where id = %s;"
#define DB_CMD_TABLE_BANK_GET   "select sum(total) from %s_%s where name = '%s';"
#define DB_CMD_TABLE_BANK_GET_NAME "select name from %s_%s group by name;"


extern int DB_Init();
extern void DB_DeInit();
extern void DB_CreateTable(char *pTable);

extern int DB_PushDataBase(char *pTable, S_DB_INFO *pDbInfo);
extern int DB_PullDataBaseAll(char *pTable, S_DB_INFO **pDbInfo, int *num);
extern int DB_DelDataBaseByIndex(char *pTable, char *date);

extern int DB_AddUser(char *name);
extern int DB_DelUser(char *name);
extern int DB_UserSetDate(char *name, char *date);
extern int DB_UserGetDate(char *name, char *date);

extern int DB_AddBank(char *pTable, char *date, char *name, char *money, char *note);
extern int DB_DelBank(char *pTable, char *date);
extern int DB_GetBank(char *pTable, char *buff);

#define DB_LOGD NOTE_LOGD
#define DB_LOGE NOTE_LOGE


#endif
