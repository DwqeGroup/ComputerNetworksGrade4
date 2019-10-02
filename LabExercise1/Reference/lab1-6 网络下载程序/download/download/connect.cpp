#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>


//winsock 2.2 library
#pragma comment(lib,"ws2_32.lib")

#define WEB_PORT 80
SOCKET sconnection;

/********************************************
功能：把字符串转换为全小写
********************************************/
void ToLowerCase(char *s)  {
	while(*s)  {
		*s=tolower(*s);
		s++;
	}//while
}

/**************************************************************
功能：从字符串src中分析出网站地址和端口，并得到用户要下载的文件
***************************************************************/
void GetHost(char * src, char * web, char * file, int * port)  {
	char * pA;
	char * pB;
	memset(web, 0, sizeof(web));
	memset(file, 0, sizeof(file));
	*port = 0;
	if(!(*src))  return;
	pA = src;
	if(!strncmp(pA, "http://", strlen("http://")))  pA = src+strlen("http://");
	else if(!strncmp(pA, "https://", strlen("https://")))  pA = src+strlen("https://");
	pB = strchr(pA, '/');
	if(pB)  {
		memcpy(web, pA, strlen(pA) - strlen(pB));
		if(pB+1)  {
			memcpy(file, pB + 1, strlen(pB) - 1);
			file[strlen(pB) - 1] = 0;
		}
	}
	else  memcpy(web, pA, strlen(pA));
	if(pB)  web[strlen(pA) - strlen(pB)] = 0;
	else  web[strlen(pA)] = 0;
	pA = strchr(web, ':');
	if(pA)  *port = atoi(pA + 1);
	else *port = 80;

	web = strtok(web,":");
}

int download(char* url, char* path, char* name)
{
	WSADATA	wsock;
	SOCKADDR_IN          serAddr;
	int                  nRet=0;
//	char url[128];
	char host_addr[256];
	char host_file[1024];
	int portnumber;
	char request[1024];
	char buffer[4003];
	char localAddress[260]="D:\\temp.html";

	 strcpy_s(localAddress, path);
	 strcat_s(localAddress, "\\");
	 strcat_s(localAddress, name);

	//初始化Winsock 2.2
	printf("\nInitialising Winsock...\n");
	if (WSAStartup(MAKEWORD(2,2),&wsock) != 0)
	{
		fprintf(stderr,"WSAStartup() failed %d\n, WSAGetLastError()");
		exit(0); 
	} 

	printf("Initialised successfully.\n");

	//创建监听socket
	printf("\nCreating TCP Socket...\n");
	if ((sconnection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		printf("Creation of socket failed %d\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	printf("TCP Socket Created successfully.\n");

//	sprintf(url, "%s", argv[1]);
//	scanf("%s",url);
	ToLowerCase(url);
	GetHost(url, host_addr, host_file, &portnumber);

	//设置SOCKADDR_IN地址结构
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(portnumber);
	//From CAsyncSocket::Connect
	serAddr.sin_addr.s_addr = inet_addr(host_addr);

	if (serAddr.sin_addr.s_addr == INADDR_NONE)
	{
		LPHOSTENT lphost;
		lphost = gethostbyname(host_addr);
		if (lphost != NULL)
			serAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
		else
		{
			WSASetLastError(WSAEINVAL);
			return 0;
		}
	}

	//连接服务端
	printf("Connecting to %s:%d...\n",
		inet_ntoa(serAddr.sin_addr), htons(serAddr.sin_port));

	if (connect(sconnection, (SOCKADDR *) &serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("connection failed with error %d\n", WSAGetLastError());
		closesocket(sconnection);
		WSACleanup();
		return 0;
	} 

	printf("connection successfully.\n");

	sprintf(request, "GET /%s HTTP/1.1\nAccept: */*\nAccept-Language: zh-cn\n\User-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\n\Host: %s:%d\nConnection: Keep-Alive\n\n", host_file, host_addr, portnumber);
	printf("%s", request);

	send(sconnection,request,strlen(request),0);
	//发送“EHLO %SMTP-Server Name%”：
	//	char szHello[PARA_BUF];
	//	sprintf(szHello,_T("EHLO %s\r\n"),ci.m_szServerName);
	//	send(sconnection,szHello,strlen(szHello),0);

	int ret;
	//解析出消息头
	ret = recv(sconnection, buffer, 4000, 0);
	if(ret<=0)
	{
		MessageBox(NULL, "接收出错", "提示信息", MB_DEFBUTTON1 | MB_ICONSTOP);
//		MessageBox(_T("recv error!"),_T("提示信息："));
		closesocket(sconnection);
		WSACleanup();
		exit(0);
	}

	buffer[ret] = '\0';
	printf("%s\n", buffer);
	while (1) {
		strcpy_s(buffer, strchr(buffer, '\n'));
		strcpy_s(buffer, buffer+1);
		if (buffer[0] == '\r' && buffer[1] == '\n') {
			strcpy_s(buffer, buffer+2);
			break;
		}
	}

	FILE* fp;
	fopen_s(&fp,localAddress, "w");
	if (fp == NULL) {
		MessageBox(NULL,"file error!","提示信息：",MB_DEFBUTTON1 | MB_ICONEXCLAMATION);
		exit(0);
	}

	fwrite(buffer, 1, strlen(buffer), fp);
	int a;
	while (1) {
		a = recv(sconnection, buffer, 1024, 0);
		if (a >= 0) {
			fwrite(buffer, 1, a, fp);
			if (a == 0) break;
		}
	}
	fclose(fp);

	printf("Closing the connection.\n");

	closesocket(sconnection);
	WSACleanup();
	MessageBox(NULL, "下载成功", "提示信息", MB_DEFBUTTON1 | MB_ICONEXCLAMATION);

	return 0;
}



//	if((recvCount=recv(sconnection,response_buf, RESPONSE_BUFFER_SIZE,0)) == SOCKET_ERROR)
//	{
//		return FALSE;
//	}
//	response_buf[recvCount]='\0';