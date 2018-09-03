#include "common.h"
#include "service_tcp.h"
#include "sqlite.h"

#undef LOG_TAG 
#define LOG_TAG "main"
#define MAX_CONNECT_USER 6

typedef struct 
{
	int  mFd;
	int  bIsConnect;
	NOTE_MSG_CONNECT mCtxInfo;	
}UserInfo;

static UserInfo mUser[MAX_CONNECT_USER];
static UserInfo * note_getFreeUser()
{
	int i = 0;
	for(i = 0; i < MAX_CONNECT_USER; i++)
	{
		if(mUser[i].bIsConnect == 0)
			return &mUser[i];
	}

	return NULL;
}

static UserInfo * note_getCurrUser(int fd)
{
	int i = 0;
	for(i = 0; i < MAX_CONNECT_USER; i++)
	{
		if(mUser[i].mFd == fd && mUser[i].bIsConnect)
			return &mUser[i];
	}

	return NULL;
}

static int note_getOneData(unsigned char *pData, NOTE_MSG_PACKET *pkt)
{
	int number = 6;
	unsigned char *pDC = pData;

	if(pData[0] != '|')
	{
		return 0;
	}

	sscanf((const char *)pData,"|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|",pkt->date, pkt->name,pkt->group,pkt->money, pkt->note);
	printf("date = %s\n",pkt->date);
	printf("name = %s\n",pkt->name);
	printf("name = %s\n",pkt->group);
	printf("money = %s\n",pkt->money);
	printf("note = %s\n",pkt->note);

	while(number)
	{
		if(pDC[0] == '|')
			number--;

		if(pDC[0] == 0)
			return 0;

		pDC++;
	}

	return pDC - pData;
}

static int note_add_user(char *name)
{
	NOTE_LOGD("%s\n",name);
	DB_AddUser(name);	
	return 0;
}

static int note_del_user(char *name)
{
	NOTE_LOGD();
	DB_DelUser(name);
	return 0;
}

static int note_add_bill(NOTE_MSG_HEARD Msg, unsigned char *pData, UserInfo *pUser)
{
	int pos = 0;
	int ret = 0;
	unsigned char *pDC = pData;
	NOTE_MSG_PACKET pkt;

	NOTE_LOGD("date = %s",pData);
	if(pData == NULL)
	{
		NOTE_LOGD("pData is NULL");
		return -1;
	}

	while(pos < Msg.s32size)
	{
		memset(&pkt, 0x00, sizeof(pkt));
		ret = note_getOneData(pDC, &pkt);
		if(ret == 0)
		{		
			NOTE_LOGE("note_getOneData size 0");

			return -1;
		}

		DB_PushDataBase(pUser->mCtxInfo.mTable, &pkt);	

		pos += ret;		
		pDC += ret;
	}

	return 0;
}

static int note_sync_bill(NOTE_MSG_HEARD Msg, unsigned char *pData, UserInfo *pUser)
{
	S_DB_INFO *pDB = NULL;

	unsigned char *pDC = NULL;
	unsigned char buff[2048] = {0};

	int length = 0;

	int i = 0;
	int j = 0;
	int len = 0;
	int total = 0;
	int index= 0;

	NOTE_LOGD("sync bill");
	DB_PullDataBaseAll(pUser->mCtxInfo.mTable, &pDB, &len);

#if 0
	for(i = 0; i < len; i++)
	{
		printf("%s %s %s %s\n", pDB[i].date, pDB[i].name, pDB[i].money, pDB[i].note);
	}
#endif

	if(len < 10)
	{
		pDC = buff + sizeof(NOTE_MSG_HEARD);
		for(i = 0; i < len; i++)
		{
			length += sprintf((char *)(pDC + length),"|%s|%s|%s|%s|%s|-",pDB[i].date, pDB[i].name, pDB[i].group, pDB[i].money, pDB[i].note);
		}

		Msg.cmd = NOTE_SYNC_BILL;
		Msg.s32size = length;
		strcpy(Msg.name, pUser->mCtxInfo.name);
		memcpy(buff, &Msg, sizeof(NOTE_MSG_HEARD));
		
		length += sizeof(NOTE_MSG_HEARD);
		
		service_tcp_SendData(pUser->mFd, buff, length);
	}
	else
	{
		total = len/10;
		index = len%10;

		for(j = 0; j < total; j++)
		{
			length = 0;
			memset(buff, 0x00, sizeof(buff));
			pDC = buff + sizeof(NOTE_MSG_HEARD);
			for(i = 0; i < 10; i++)
			{
				length += sprintf((char *)(pDC + length),"|%s|%s|%s|%s|%s|-",pDB[j * 10 + i].date, pDB[j * 10 + i].name, pDB[j * 10 + i].group, pDB[j * 10 + i].money, pDB[j * 10 + i].note);
			}

			Msg.cmd = NOTE_SYNC_BILL;
			Msg.s32size = length;
			strcpy(Msg.name, pUser->mCtxInfo.name);
			memcpy(buff, &Msg, sizeof(NOTE_MSG_HEARD));

			length += sizeof(NOTE_MSG_HEARD);
			
			service_tcp_SendData(pUser->mFd, buff, length);	
		}

		if(index == 0)
			goto exit;

		length = 0;
		memset(buff, 0x00, sizeof(buff));
		pDC = buff + sizeof(NOTE_MSG_HEARD);
		for(i = 0; i < index; i++)
		{
			length += sprintf((char *)(pDC + length),"|%s|%s|%s|%s|%s|-",pDB[total * 10 + i].date, pDB[total * 10 + i].name, pDB[total * 10 + i].group, pDB[total * 10 + i].money, pDB[total * 10 + i].note);
		}

		Msg.cmd = NOTE_SYNC_BILL;
		Msg.s32size = length;
		strcpy(Msg.name, pUser->mCtxInfo.name);
		memcpy(buff, &Msg, sizeof(NOTE_MSG_HEARD));

		length += sizeof(NOTE_MSG_HEARD);
		
		service_tcp_SendData(pUser->mFd, buff, length);	
	}

exit:
	free(pDB);

	return 0;
}

static int note_del_bill(NOTE_MSG_HEARD Msg, unsigned char *pData, UserInfo *pUser)
{
	char date[16] = {0};
	strcpy(date, (char *)pData);

	NOTE_LOGD("index = %s",date);
	return DB_DelDataBaseByIndex(pUser->mCtxInfo.mTable, date);	
}

static int note_user_connect(NOTE_MSG_HEARD Msg, unsigned char *pData, UserInfo *pUser)
{
	char name[16] = {0};;
	memcpy(&(pUser->mCtxInfo), pData, sizeof(NOTE_MSG_CONNECT));
	printf("%s %s %s %s\n",pUser->mCtxInfo.mID, 
			pUser->mCtxInfo.mTable,
			pUser->mCtxInfo.name,pUser->mCtxInfo.upDate);
	if(DB_UserGetDate(pUser->mCtxInfo.name, name) == 0)
	{
		pUser->bIsConnect = 1;
		DB_CreateTable(pUser->mCtxInfo.mTable);
		NOTE_LOGD("%s is connect",pUser->mCtxInfo.name);
		return 0;
	}
	else
	{
		NOTE_LOGD("%s is not connect",pUser->mCtxInfo.name);
		pUser->bIsConnect = 0;
		return -1;
	}
}

static int note_user_disconnect(UserInfo *pUser)
{

	NOTE_LOGD("%s is disconnect",pUser->mCtxInfo.name);
	memset(&pUser->mCtxInfo, 0x00, sizeof(NOTE_MSG_CONNECT));
	pUser->bIsConnect = 0;
	pUser->mFd = -1;

	return 0;
}


static int note_add_bank(NOTE_MSG_HEARD Msg, unsigned char *pData, UserInfo *pUser)
{
	char date[16] = {0};
	char name[16] = {0};
	char money[16] = {0};
	char note[256] = {0};

	sscanf((const char *)pData,"|%[^|]|%[^|]|%[^|]|%[^|]|",date, name, money,note);
	printf("date = %s, name = %s, money = %s, note = %s\n",date, name, money, note);
	DB_AddBank(pUser->mCtxInfo.mTable,date, name, money, note);

	return 0;
}

static int note_get_bank(NOTE_MSG_HEARD Msg, unsigned char *pData, UserInfo *pUser)
{
	char buff[1024] = {0};

	NOTE_LOGD();
	Msg.cmd = NOTE_GET_BANK;
	Msg.s32size = DB_GetBank(pUser->mCtxInfo.mTable, buff + sizeof(NOTE_MSG_HEARD));
	strcpy(Msg.name, pUser->mCtxInfo.name);
	memcpy(buff, &Msg, sizeof(NOTE_MSG_HEARD));

	service_tcp_SendData(pUser->mFd, (unsigned char *)buff, Msg.s32size + sizeof(NOTE_MSG_HEARD));

	return 0;
}

static int note_del_bank(NOTE_MSG_HEARD Msg, unsigned char *pData, UserInfo *pUser)
{
	char date[16] = {0};
	strcpy(date, (char *)pData);
	NOTE_LOGD("index : %s",date);
	DB_DelBank(pUser->mCtxInfo.mTable,date);
	return 0;
}

static void note_DisposeData(SERVICE_MSG *pMsg)
{
	NOTE_MSG_HEARD MsgHdrC;
	int length = 0;
	UserInfo *pUser = NULL;
	int ACK_CMD = 0;
	int currPos = 0;

	while(length < pMsg->u32size)
	{
		pMsg->pBuff += currPos;
		memset(&MsgHdrC, 0x00, sizeof(NOTE_MSG_HEARD));
		memcpy(&MsgHdrC, pMsg->pBuff, sizeof(NOTE_MSG_HEARD));

		//printf("Hdr.cmd = %d Hdr.size = %d Hdr.name = %s\n",MsgHdrC.cmd, MsgHdrC.s32size, MsgHdrC.name);

		if(MsgHdrC.s32size > pMsg->u32size - currPos)
		{
			printf("head size = %d\n",MsgHdrC.s32size);
			break;
		}

		switch(MsgHdrC.cmd)
		{
			case NOTE_ADD_USER:
				{
					ACK_CMD = note_add_user(MsgHdrC.name);
				}
				break;

			case NOTE_DEL_USER:
				{
					ACK_CMD = note_del_user(MsgHdrC.name);
				}
				break;

			case NOTE_ADD_BILL:
				{
					pUser = note_getCurrUser(pMsg->fd);
					if(pUser)
						ACK_CMD = note_add_bill(MsgHdrC, pMsg->pBuff + sizeof(NOTE_MSG_HEARD), pUser);	
				}
				break;

			case NOTE_SYNC_BILL:
				{
					pUser = note_getCurrUser(pMsg->fd);
					if(pUser)
						ACK_CMD = note_sync_bill(MsgHdrC, pMsg->pBuff + sizeof(NOTE_MSG_HEARD), pUser);	
				}
				break;

			case NOTE_CONNECT:
				{
					pUser = note_getFreeUser();
					if(pUser)
					{
						pUser->mFd = pMsg->fd;
						ACK_CMD = note_user_connect(MsgHdrC, pMsg->pBuff + sizeof(NOTE_MSG_HEARD), pUser);	
					}
				}
				break;

			case NOTE_ADD_BANK:
				{
					pUser = note_getCurrUser(pMsg->fd);
					if(pUser)
						ACK_CMD = note_add_bank(MsgHdrC, pMsg->pBuff + sizeof(NOTE_MSG_HEARD), pUser);	

				}
				break;

			case NOTE_GET_BANK:
				{
					pUser = note_getCurrUser(pMsg->fd);
					if(pUser)
						ACK_CMD = note_get_bank(MsgHdrC, pMsg->pBuff + sizeof(NOTE_MSG_HEARD), pUser);	


				}
				break;
			case NOTE_DEL_BANK:
				{
					pUser = note_getCurrUser(pMsg->fd);
					if(pUser)
						ACK_CMD = note_del_bank(MsgHdrC, pMsg->pBuff + sizeof(NOTE_MSG_HEARD), pUser);	

				}
				break;
			case NOTE_DEL_BILL:
				{
					pUser = note_getCurrUser(pMsg->fd);
					if(pUser)
						ACK_CMD = note_del_bill(MsgHdrC, pMsg->pBuff + sizeof(NOTE_MSG_HEARD), pUser);	

				}
				break;

			default:
				ACK_CMD = ACK_CMD;
				break;
		}
		currPos = MsgHdrC.s32size + sizeof(NOTE_MSG_HEARD);
		length +=currPos;
	}
}

void service_tcp_Callback(SERVICE_TCP_STATE cmd, void *arg)
{
	switch(cmd)
	{
		case SERVICE_TCP_CONNECT:
			{
				ClientAddr *sock_addr = (ClientAddr *)arg;
				NOTE_LOGD("new client connect index = %d, fd = %d, ip = %s", sock_addr->index, sock_addr->fd, sock_addr->ip);
			}
			break;

		case SERVICE_TCP_DISCONNECT:
			{
				ClientAddr *sock_addr = (ClientAddr *)arg;
				NOTE_LOGD("new client disconnect index = %d, fd = %d, ip = %s", sock_addr->index, sock_addr->fd, sock_addr->ip);
				UserInfo *pUser = note_getCurrUser(sock_addr->fd);

				if(pUser != NULL)
					note_user_disconnect(pUser);
			}
			break;

		case SERVICE_TCP_RECV_MSG:
			{
				SERVICE_MSG *msg = (SERVICE_MSG *)arg;
				//NOTE_LOGD("recv client msg:%s",msg->pBuff); 
				note_DisposeData(msg);
			}
			break;

		case SERVICE_TCP_BUFF:
		default:
			break;
	}
}

int main()
{
	service_tcp_init();

	service_tcp_SetCallback(service_tcp_Callback);

	DB_Init();

	while(1) sleep(20);
}
