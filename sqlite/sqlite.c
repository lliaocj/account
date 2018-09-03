#include <sqlite.h>

static sqlite3 *hDB = NULL;

#undef  LOG_TAG
#define LOG_TAG "db"

static int DB_TableIsExist(sqlite3 *pDB, char *pTable)
{
	int num = 0;
	int Row,Col,ret;
	char command[128] = {0};	
	char **ppResult = NULL;
	char *Error = NULL;

	sprintf(command, DB_CMD_TABLE_IS_EXITS, pTable);

	//DB_LOGD("sqlite %s\n",command);
	ret = sqlite3_get_table(pDB, command, &ppResult, &Row, &Col, &Error);
	if(ret != 0)
	{
		return 0;	
	}

	num = atoi(ppResult[1]);

	sqlite3_free_table(ppResult);
	sqlite3_free(Error);

	//DB_LOGD("num = %d\n",num);

	return num;
}

int DB_Init()
{
	int ret = 0;
	char sql[128] = {0};

	if(!hDB)
	{
		ret = sqlite3_open(DB_FILE_PATH, &hDB);
		if(ret != 0)
		{
			DB_LOGE("open %s error errno(%d)",DB_FILE_PATH,ret);
			return ret;
		}
	}

	sqlite3_exec(hDB, DB_CMD_TABLE_USER_CREATE, NULL, NULL, NULL);	
}

void DB_DeInit()
{
	if(hDB)
	{
		sqlite3_close(hDB);
		hDB = NULL;
	}
}

void DB_CreateTable(char *pTable)
{
	char table[128] = {0};
	if(!strlen(pTable))
	{
		DB_LOGE("this pTable is " " ");
		return ;
	}

	char command[1024] = {0};
	sprintf(table,"%s_%s",DB_BANK_TABLE,pTable);
	sprintf(command, DB_CMD_TABLE_BANK_CREATE, DB_BANK_TABLE ,pTable);
	if(DB_TableIsExist(hDB,table) == 0)
		sqlite3_exec(hDB, command, NULL, NULL, NULL);	

	memset(command, 0x00, sizeof(command));
	sprintf(command, DB_CMD_TABLE_CREATE, pTable);
	if(DB_TableIsExist(hDB, pTable) == 0)
		sqlite3_exec(hDB, command, NULL, NULL, NULL);
}

int DB_PushDataBase(char *pTable, S_DB_INFO *pDbInfo)
{
	char command[1024] = {0};
	if(pTable == NULL)
	{
		DB_LOGE("this para error");
		return -1;
	}

	if(hDB == NULL)
	{
		DB_LOGE("this db not open");
		return -1;
	}
	
	if(pDbInfo == NULL)
	{
		DB_LOGD("no data insert");
		return 0;
	}

	//insert data
	memset(command, 0x00, sizeof(command));
	sprintf(command, DB_CMD_TABLE_INSERT,pTable, pDbInfo->date, pDbInfo->name, pDbInfo->group, pDbInfo->money, pDbInfo->note);
	sqlite3_exec(hDB, command, NULL, NULL, NULL);

	return 0;
}

int DB_PullDataBaseAll(char *pTable, S_DB_INFO **pDbInfo, int *num)
{
	int i,j;
	int Row,Col,ret;
	char **ppResult = NULL;
	char *Error = NULL;
	S_DB_INFO *pQury = NULL;

	char command[128] = {0};
	if(hDB == NULL)
	{
		DB_LOGE("this db not open");
		return -1;
	}

	if(!strlen(pTable))
	{
		DB_LOGE("this pTable is " " ");
		return -1;
	}

	sprintf(command, DB_CMD_TABLE_SELECT_ALL, pTable);

	ret = sqlite3_get_table(hDB, command, &ppResult, &Row, &Col, &Error);
	if(ret != 0)
	{
		return 0;	
	}

	pQury = (S_DB_INFO *)malloc(sizeof(S_DB_INFO) * Row);
	*num = Row;

	for(i = 1; i <= Row; i++)
	{
		strcpy(pQury[i - 1].date, ppResult[i * Col + 0]);
		strcpy(pQury[i - 1].name, ppResult[i * Col + 1]);
		strcpy(pQury[i - 1].group, ppResult[i * Col + 2]);
		strcpy(pQury[i - 1].money, ppResult[i * Col + 3]);
		strcpy(pQury[i - 1].note, ppResult[i * Col + 4]);
	}

	sqlite3_free_table(ppResult);
	sqlite3_free(Error);

	*pDbInfo = pQury;

	return 0;
}

int DB_DelDataBaseByIndex(char *pTable, char *date)
{
	char command[1024] = {0};
	if(pTable == NULL)
	{
		DB_LOGE("this para error");
		return -1;
	}

	if(hDB == NULL)
	{
		DB_LOGE("this db not open");
		return -1;
	}

	sprintf(command, DB_CMD_TABLE_DEL, pTable, date);
	return sqlite3_exec(hDB, command, NULL, NULL, NULL);
}

int DB_AddUser(char *name)
{
	char command[128] = {0};
	if(hDB == NULL)
	{
		DB_LOGE("this db not open");
		return -1;
	}

	sprintf(command, DB_CMD_TABLE_USER_ADD, name, "0");
	sqlite3_exec(hDB, command, NULL, NULL, NULL);	
}

int DB_DelUser(char *name)
{
	char command[128] = {0};
	if(hDB == NULL)
	{
		DB_LOGE("this db not open");
		return -1;
	}

	sprintf(command, DB_CMD_TABLE_USER_DEL, name);
	sqlite3_exec(hDB, command, NULL, NULL, NULL);	
}

int DB_UserSetDate(char *name, char *date)
{
	char command[128] = {0};
	if(hDB == NULL)
	{
		DB_LOGE("this db not open");
		return -1;
	}

	sprintf(command, DB_CMD_TABLE_USER_SET_DATE, name, date);
	sqlite3_exec(hDB, command, NULL, NULL, NULL);	
}

int DB_UserGetDate(char *name, char *date)
{
	int Row,Col,ret;
	char **ppResult = NULL;

	char command[128] = {0};
	if(hDB == NULL)
	{
		DB_LOGE("this db not open");
		return -1;
	}

	sprintf(command, DB_CMD_TABLE_USER_GET_DATE, name);

	ret = sqlite3_get_table(hDB, command, &ppResult, &Row, &Col, NULL);
	if(ret != 0)
	{
		DB_LOGE("sqlite get table error");
		return -1;	
	}

	if(Row == 1)
		strcpy(date, ppResult[1]);
	else
	{
		sqlite3_free_table(ppResult);
		return -1;
	}

	sqlite3_free_table(ppResult);
}

int DB_AddBank(char *pTable,char *date, char *name, char *money, char *note)
{
	char command[128] = {0};
	if(hDB == NULL)
	{
		DB_LOGE("this db not open");
		return -1;
	}

	sprintf(command, DB_CMD_TABLE_BANK_ADD, DB_BANK_TABLE, pTable,date, name, money, note);
	sqlite3_exec(hDB, command, NULL, NULL, NULL);	
}

int DB_DelBank(char *pTable,char *date)
{
	char command[128] = {0};
	if(hDB == NULL)
	{
		DB_LOGE("this db not open");
		return -1;
	}

	sprintf(command, DB_CMD_TABLE_BANK_DEL, DB_BANK_TABLE, pTable, date); 
	sqlite3_exec(hDB, command, NULL, NULL, NULL);	
}

int DB_GetBank(char *pTable, char *buff)
{
	int i = 0,j = 0;
	int length = 0;
	int Row,Row2,Col,Col2,ret;
	char command[128] = {0};	
	char **ppResult = NULL;
	char *Error = NULL;

	char **ppResult2 = NULL;

	if(hDB == NULL)
	{
		DB_LOGE("this db not open");
		return -1;
	}

	sprintf(command, DB_CMD_TABLE_BANK_GET_NAME, DB_BANK_TABLE, pTable);
	ret = sqlite3_get_table(hDB, command, &ppResult, &Row, &Col, NULL);
	if(ret != 0)
	{
		return -1;
	}

	for(i = 1; i <= Row; i++)
	{
		memset(command, 0x00, sizeof(command));
		sprintf(command, DB_CMD_TABLE_BANK_GET, DB_BANK_TABLE, pTable, ppResult[i * Col + 0]);
		ret = sqlite3_get_table(hDB, command, &ppResult2, &Row2, &Col2, NULL);
		if(ret != 0)
		{
			sqlite3_free_table(ppResult);
			return -1;
		}

		length += sprintf(buff + length, "%s:%s|", ppResult[i * Col + 0], ppResult2[1]);	
	}	

	sqlite3_free_table(ppResult);
	sqlite3_free_table(ppResult2);

	return length;
}
