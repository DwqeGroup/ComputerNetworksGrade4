
#include <WinSock2.h>

#include <windows.h>

#include "sizes.h"

#include <tchar.h>

#include <iostream>

 

#pragma comment(lib, "ws2_32.lib")

 

//读取回复报文

void do_read_rspns(SOCKET fd, RspnsPacket *ptr)

{

	int count = 0;

	int size = sizeof(RspnsPacket);

	while (count < size)

	{

		int nRead = recv(fd, (char *)ptr + count, size - count, 0);

		if (nRead <= 0)

		{

			printf("读取服务器的回复失败！\n");

			closesocket(fd);

			exit(1);

		}

		count += nRead;

	}

}

 

//发送命令报文

void do_write_cmd(SOCKET fd, CmdPacket *ptr)

{

	int size = sizeof(CmdPacket);

	int flag = send(fd, (char *)ptr, size, 0);

	if (flag == SOCKET_ERROR)

	{

		printf("给服务器发送命令失败！\n");

		closesocket(fd);

		WSACleanup();

		exit(1);

	}

}

 

//创建数据连接套接字并进入侦听状态

SOCKET create_data_socket()

{

	SOCKET sockfd;

	struct sockaddr_in my_addr;

	//创建用于数据连接的套接字

	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)

	{

		printf("创建用于数据连接的套接字失败！\n");

		WSACleanup();

		exit(1);

	}

	my_addr.sin_family = AF_INET;

	my_addr.sin_port = htons(DATA_PORT);

	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	memset(&(my_addr.sin_zero), 0, sizeof(my_addr.sin_zero));

 

	//绑定

	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == SOCKET_ERROR)

	{

		int err = WSAGetLastError();

		printf("绑定地址失败，错误代码：%d\n", err);

		closesocket(sockfd);

		WSACleanup();

		exit(1);

	}

 

	//侦听数据连接请求

	if (listen(sockfd, 1) == SOCKET_ERROR)

	{

		printf("监听数据连接失败！\n");

		closesocket(sockfd);

		WSACleanup();

		exit(1);

	}

	return sockfd;

}

 

//处理list命令

void list(SOCKET sockfd)

{

	int sin_size;

	int nRead;

	CmdPacket cmd_packet;

	SOCKET newsockfd, data_sockfd;

	struct sockaddr_in their_add;

	char data_buf[DATA_BUFSIZE];

 

	//创建数据连接

	newsockfd = create_data_socket();

	//构建命令报文并发送至服务器

	cmd_packet.cmdid = LS;//没有参数

	do_write_cmd(sockfd, &cmd_packet);

	sin_size = sizeof(struct sockaddr_in);

	//接受服务器的数据连接请求

	if ((data_sockfd = accept(newsockfd, (struct sockaddr*)&their_add, &sin_size)) == INVALID_SOCKET)

	{

		printf("获取文件列表失败！\n");

		closesocket(newsockfd);

		closesocket(sockfd);

		WSACleanup();

		exit(1);

	}

 

	//每次读到多少数据就显示多少，直到数据连接断开

	while (true)

	{

		nRead = recv(data_sockfd, data_buf, DATA_BUFSIZE - 1, 0);

		if (nRead == SOCKET_ERROR)

		{

			printf("读取服务器回复失败！\n");

			closesocket(data_sockfd);

			closesocket(newsockfd);

			closesocket(sockfd);

			WSACleanup();

			exit(1);

		}

 

		if (nRead == 0)//数据读取结束

			break;

 

		//显示数据

		data_buf[nRead] = '\0';

		printf("%s", data_buf);

 

	}

	closesocket(data_sockfd);

	closesocket(newsockfd);

}

//处理pwd命令：

void pwd(int sockfd)

{

	CmdPacket cmd_packet;

	RspnsPacket rspns_packet;

 

	cmd_packet.cmdid = PWD;

	//发送命令报文并读取回复：

	do_write_cmd(sockfd, &cmd_packet);

	do_read_rspns(sockfd, &rspns_packet);

	printf("%s\n", rspns_packet.text);

}

 

//处理cd命令:

void cd(int sockfd)

{

	CmdPacket cmd_packet;

	RspnsPacket rspns_packet;

 

 

	cmd_packet.cmdid = CD;

	scanf("%s", cmd_packet.param);

 

	//发送命令报文并读取回复：

	do_write_cmd(sockfd, &cmd_packet);

	do_read_rspns(sockfd, &rspns_packet);

	if (rspns_packet.rspnsid == ERR)

		printf("%s", rspns_packet.text);

}

 

 

//处理down命令，即下载文件：

void get_file(SOCKET sockfd)

{

	FILE *fd;

	char data_buf[DATA_BUFSIZE];

 

	CmdPacket cmd_packet;

	RspnsPacket rspns_packet;

 

	SOCKET newsockfd, data_sockfd;

	struct sockaddr_in their_addr;

	int sin_size;

	int count;

 

	//设置命令报文：

	cmd_packet.cmdid = DOWN;

	scanf("%s", cmd_packet.param);

 

	//打开或者创建本地文件以供写数据：

	fd = fopen(cmd_packet.param, "wb");//使用二进制方程

	if (fd == NULL)

	{

		printf("打开文件%s来写入失败！\n", cmd_packet.param);

		return;

	}

 

	//创建数据连接并侦听服务器的连接请求：

	newsockfd = create_data_socket();

 

	//发送报文请求：

	do_write_cmd(sockfd, &cmd_packet);

 

	//读取回复报文：

	do_read_rspns(sockfd, &rspns_packet);

	if (rspns_packet.rspnsid == ERR)

	{

		printf("%s", rspns_packet.text);

		closesocket(newsockfd);

 

		fclose(fd);

		//删除文件:

		DeleteFile(cmd_packet.param);

		return;

	}

 

	sin_size = sizeof(struct sockaddr_in);

	//等待接受服务器的连接请求

	if ((data_sockfd = accept(newsockfd, (struct sockaddr *)&their_addr, &sin_size)) == INVALID_SOCKET)

	{

		printf("获取文件失败！\n");

		closesocket(newsockfd);

 

		fclose(fd);

		//删除文件：

		DeleteFile(cmd_packet.param);

		return;

	}

 

	//循环读取网络数据并写入文件：

	while ((count = recv(data_sockfd, data_buf, DATA_BUFSIZE, 0)) > 0)

		fwrite(data_buf, sizeof(char), count, fd);

 

	closesocket(data_sockfd);

	closesocket(newsockfd);

	fclose(fd);

}

 

//处理put命令，即上传文件

void put_file(SOCKET sockfd)

{

	FILE *fd;

	CmdPacket cmd_packet;

	RspnsPacket rspns_packet;

	char data_buf[DATA_BUFSIZE];

 

	SOCKET newsockfd, data_sockfd;

	struct sockaddr_in their_addr;

	int sin_size;

	int count;

	cmd_packet.cmdid = UP;

	scanf("%s", cmd_packet.param);

 

	//打开本地文件用于读取数据

	fd = fopen(cmd_packet.param, "rb");

	if (fd == NULL)

	{

		printf("打开文件%s来读取数据失败！\n", cmd_packet.param);

		return;

	}

 

	//创建数据连接套接字并进入侦听状态；

	newsockfd = create_data_socket();

 

	//发送命令报文

	do_write_cmd(sockfd, &cmd_packet);

 

	//读取回复报文

	do_read_rspns(sockfd, &rspns_packet);

	if (rspns_packet.rspnsid == ERR)

	{

		printf("%s", rspns_packet.text);

		closesocket(newsockfd);

		fclose(fd);

		return;

	}

 

	sin_size = sizeof(struct sockaddr_in);

	//准备接受数据连接

	if ((data_sockfd = accept(newsockfd, (struct sockaddr *)&their_addr, &sin_size)) == INVALID_SOCKET)

	{

		printf("上传文件传输错误！\n");

		closesocket(newsockfd);

		fclose(fd);

		return;

	}

	//循环从文件中读取数据并发给服务器

	while (true)

	{

		count = fread(data_buf, sizeof(char), DATA_BUFSIZE, fd);

		send(data_sockfd, data_buf, count, 0);

		if (count < DATA_BUFSIZE)//数据已经读完或者发生cuowu

			break;

	}

 

	closesocket(data_sockfd);

	closesocket(newsockfd);

	fclose(fd);

}

 

//处理退出命令

void quit(int sockfd)

{

	CmdPacket cmd_packet;

	RspnsPacket rspns_packet;

 

	cmd_packet.cmdid = QUIT;

	do_write_cmd(sockfd, &cmd_packet);

	do_read_rspns(sockfd, &rspns_packet);

	printf("%s", rspns_packet.text);

	getchar();

}

 

int main()

{

	SOCKET sockfd;

	struct sockaddr_in their_addr;

	char cmd[10];

	RspnsPacket rspns_packet;

 

	WORD wVersionRequested;

	WSADATA wsaData;

	int err;

	wVersionRequested = MAKEWORD(2, 2);

	//Winsock初始化

	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0)

	{

		printf("WinSock初始化失败！\n");

		return 0;

	}

 

	//确认WindSock DLL的版本是2.2

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)

	{

		printf("WindSock版本不是2.2！\n");

		WSACleanup();

		return 0;

	}

 

	//创建用于控制谅解的socket

	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sockfd == INVALID_SOCKET)

	{

		printf("创建套接字失败！\n");

		WSACleanup();

		exit(1);

	}

	their_addr.sin_family = AF_INET;

	their_addr.sin_port = htons(CMD_PORT);

	their_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	memset(&(their_addr.sin_zero), 0, sizeof(their_addr.sin_zero));

 

	//连接服务器

	if (connect(sockfd, (struct sockaddr*)&their_addr, sizeof(struct sockaddr)) == SOCKET_ERROR)

	{

		printf("连接服务器失败！\n");

		closesocket(sockfd);

		WSACleanup();

		exit(1);

	}

 

	//连接成功后，首先接受服务器发回的消息

	do_read_rspns(sockfd, &rspns_packet);

	printf("%s", rspns_packet.text);

 

	//主循环：读取用户输入并分配执行

	while (true)

	{

		scanf("%s", cmd);

		switch (cmd[0])

		{

		case 'l'://处理List命令

			list(sockfd);

			break;

		case 'p'://处理pwd命令

			pwd(sockfd);

			break;

		case 'c'://处理cd命令

			cd(sockfd);

			break;

		case 'd'://处理down命令

			get_file(sockfd);

			break;

		case 'u'://处理up命令

			put_file(sockfd);

			break;

		case 'q'://处理quit命令

			quit(sockfd);

			break;

		default:

			printf("不存在的命令！\n");

			break;

		}

		if (cmd[0] == 'q')

			break;

	}

	WSACleanup();

}

