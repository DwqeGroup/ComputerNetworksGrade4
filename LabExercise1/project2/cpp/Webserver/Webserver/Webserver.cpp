// Webserver.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WinSock.h>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <string>
#pragma comment(lib,"ws2_32.lib")
#pragma warning( disable : 4996) 
using namespace std;

void geturl(char * request, char * url) {
	char *p;
	char *r;
	p = request;

	while (*p != ' ')
		p++;
	p += 2;

	r = p;
	while (*r != ' ')
		r++;
	*r = '\0';

	strcpy(url, p);

	p = url;
	while (*p != '\0') {
		if (*p == '/')
			*p = '\\';
		p++;
	}
}

void reply(char * request, char * replybuf) {

	char url[1024] = { 0 };
	geturl(request, url); //得到url
	cout << url << endl;

	FILE *fp;
	if ((fp = fopen(url, "r")) == NULL) {
		strcpy(replybuf, "HTTP/1.1 404 Not Found\r\n\r\n");
	}
	else {
		char tempstring[1024];
		char ch;
		int i = 0;
		while ((ch = fgetc(fp)) != EOF) {
			tempstring[i] = ch;
			i++;
		}
		tempstring[i] = '\0';
		//		printf("HTTP/1.1 200 OK\nContent-length: 100\nContent-Type: text/html\n\n%s",tempstring);
		sprintf(replybuf, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n%s", tempstring);

		//		printf("HTTP/1.1 202 Accepted\r\n\r\n%s",tempstring);
		//	sprintf(replybuf,"HTTP/1.1 202 Accepted\r\n\r\n%s",tempstring);
	}
}


int main()
{
	WSAData wsaData;
	if (WSAStartup(0x101, &wsaData)) {
		cout << "start error!" << endl;
	}
	else {
		cout << "start ok!" << endl;
	}

	SOCKET listenSock, newsock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET) {
		cout << "listen socket create error!" << endl;
	}
	else {
		cout << "listen socket create succeed!" << endl;
	}

	struct sockaddr_in srvAddr, client;
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");
	srvAddr.sin_port = htons(8088);

	if (bind(listenSock, (sockaddr*)&srvAddr, sizeof(srvAddr)) == SOCKET_ERROR) {
		cout << "bind error!" << endl;
	}
	else {
		cout << "bind ok!" << endl;
	}

	if (listen(listenSock, 1) == SOCKET_ERROR) {
		cout << "listen error!" << endl;
	}
	else {
		cout << "listening..." << endl;
	}

	//////////////////////////////////////////////////////////////////////////

	char request[1024] = { 0 };
	char replybuf[1024] = { 0 };
	while (1)
	{
		int len = sizeof(client);
		newsock = accept(listenSock, (sockaddr*)&client, &len);
		if (newsock == INVALID_SOCKET) {
			cout << "accept error" << endl;
		}
		else {
			cout << "accept ok!" << endl;
		}

		recv(newsock, request, 1024, 0);
		cout << request << endl;
		reply(request, replybuf);
		cout << replybuf << endl;
		send(newsock, replybuf, strlen(replybuf), 0);

		closesocket(newsock);

		cout << "listening..." << endl;
	}

	closesocket(listenSock);
	WSACleanup();


	return 0;
}