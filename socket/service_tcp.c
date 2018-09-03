#include "service_tcp.h"
 
#undef LOG_TAG 
#define LOG_TAG "service_tcp"

static pthread_t pid;
static int sockfd = 0;
static funListen service_tcp_Callback;

static ClientAddr clien_addr[TCP_MAX_CLIENT];

static int service_tcp_GetFreeClient(void)
{
	int i = 0;
	for(i = 0; i < sizeof(clien_addr)/sizeof(clien_addr[0]); i++)
		if(clien_addr[i].use == 0)
			break;

	if(i == sizeof(clien_addr)/sizeof(clien_addr[0]))
		return -1;

	return i; 
}

static void *service_tcp_Processor(void *arg)
{
	int ret = 0;
	int i = 0;
	int  client_fd = 0;	
	int  max_fd = 0;
	fd_set   read_fds , select_fds;
	struct sockaddr_in c_SockAddr;
	struct timeval timeout;
	unsigned char buff[1024] = {0};
	socklen_t  c_len = sizeof(c_SockAddr);

	memset(&timeout, 0x00, sizeof(timeout));

	FD_ZERO(&read_fds);	
	FD_SET(sockfd, &read_fds);	
	max_fd = sockfd;

	while(1)
	{
		FD_ZERO(&select_fds);
		select_fds = read_fds;

		ret = select(max_fd + 1, &select_fds, NULL, NULL, NULL);
		if(ret < 0)
		{
			continue;
		}

		if(FD_ISSET(sockfd, &select_fds))
		{
			memset(&c_SockAddr, 0x00, sizeof(c_SockAddr));
			client_fd = accept(sockfd, (struct sockaddr *)(&c_SockAddr), &c_len);
			if(client_fd < 0)
			{
				continue;
			}

			if(client_fd > max_fd)
				max_fd = client_fd;
				
			FD_SET(client_fd, &read_fds);

			i = service_tcp_GetFreeClient();
			if(i < 0)
			{
				NOTE_LOGE("connect is full");
				continue;
			}

			clien_addr[i].fd = client_fd;
			clien_addr[i].use = 1;
			clien_addr[i].index = i;
			inet_ntop(AF_INET, &c_SockAddr.sin_addr, clien_addr[i].ip, INET_ADDRSTRLEN);

			if(service_tcp_Callback)
				service_tcp_Callback(SERVICE_TCP_CONNECT, (void *)&clien_addr[i]);
		}
		else
		{
			for(i = 0; i < TCP_MAX_CLIENT; i++)
			{
				if(clien_addr[i].use == 0)
					continue;

				if(FD_ISSET(clien_addr[i].fd,&select_fds))
				{
					memset(buff, 0x00, sizeof(buff));
					int length = read(clien_addr[i].fd, buff, sizeof(buff));
					if(length > 0)
					{
						SERVICE_MSG msg;	
						msg.fd = clien_addr[i].fd;
						msg.pBuff = buff;
						msg.u32size = length;
						if(service_tcp_Callback)
							service_tcp_Callback(SERVICE_TCP_RECV_MSG, (void *)&msg);
					}
					else
					{			
						if(service_tcp_Callback)
							service_tcp_Callback(SERVICE_TCP_DISCONNECT, (void *)&clien_addr[i]);
						clien_addr[i].use = 0;
						close(clien_addr[i].fd); 
						FD_CLR(clien_addr[i].fd, &read_fds);
						clien_addr[i].fd = -1;
					}
				}
			}
		}	
	}

	return NULL;
}

int service_tcp_init()
{
	int ret = 0;
	struct sockaddr_in SockAddr;
	int s32Socket_opt_value = 1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		NOTE_LOGE("create socket error sockdf = %d",sockfd);

		return -1;
	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &s32Socket_opt_value, sizeof(int)) == -1)
    {   
        NOTE_LOGE("Setsockopt Fail!\n");
        return -1;
    }
	memset(&SockAddr, 0x00, sizeof(SockAddr));
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(TCP_PORT);
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(sockfd, (struct sockaddr *)(&SockAddr), sizeof(SockAddr));
	if(ret < 0)
	{
		NOTE_LOGE("socket bind error %d", ret);

		goto sock_close;
	}

	ret = listen(sockfd, 10);
	if(ret < 0)
	{
		NOTE_LOGE("socket listen error %d",ret);

		goto sock_close;
	}	

	usleep(200);

	pthread_create(&pid, NULL, service_tcp_Processor, NULL);

	return 0;

sock_close:
	close(sockfd);

	return -1;
}

void service_tcp_SetCallback(funListen pFunction)
{
	if(pFunction != NULL)
		service_tcp_Callback = pFunction;	
}

int service_tcp_SendData(int SockFd, unsigned char *pBuff, int u32size)
{
	if(!pBuff || u32size == 0)
	{
		NOTE_LOGE("input para error");
		
		return -1;
	}

	return send(SockFd, pBuff, u32size, 0);

}
