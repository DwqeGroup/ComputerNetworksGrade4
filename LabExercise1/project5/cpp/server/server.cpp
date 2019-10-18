
#include <WinSock2.h>

#include "sizes.h"

#include <iostream>

 

#pragma comment(lib, "ws2_32.lib")

 

//创建线程时传递的数据结构,内含控制连接套接字和客户端地址信息:

struct threadData {

	SOCKET tcps;

	sockaddr_in clientaddr;

};

 

//全局函数声明:

//FTP初始化,创建一个侦听套接字:

int InitFTP(SOCKET *pListenSock);

int InitDataSocket(SOCKET *pDatatcps, SOCKADDR_IN *pClientAddr);

int ProcessCmd(SOCKET tcps, CmdPacket* pCmd, SOCKADDR_IN *pClientAddr);

int SendRspns(SOCKET tcps, RspnsPacket* prspns);

int RecvCmd(SOCKET tcps, char* pCmd);

int SendFileList(SOCKET datatcps);

int SendFileRecord(SOCKET datatcps, WIN32_FIND_DATA* pfd);

int SendFile(SOCKET datatcps, FILE* file);

int RecvFile(SOCKET datatcps, char* filename);

int FileExists(const char *filename);

 

//线程函数,参数包括相应控制连接的套接字:

DWORD WINAPI ThreadFunc(LPVOID lpParam) {

	SOCKET tcps;

	sockaddr_in clientaddr;

	tcps = ((struct threadData *)lpParam)->tcps;

	clientaddr = ((struct threadData *)lpParam)->clientaddr;

	printf("socket的编号是：%u.\n", tcps);

 

	//发送回复报文给客户端,内含命令使用说明:

	printf("Serve client %s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	RspnsPacket rspns = { OK,

			"欢迎进入FTP综合应用系统!\n"

			"你可以使用的命令:\n"

			"ls\t<展示当前目录下的文件(夹)，无需参数>\n"

			"pwd\t<展示当前目录的绝对路径，无需参数>\n"

			"cd\t<切换到指定目录，参数为路径>\n"

			"down\t<下载文件，参数为文件名>\n"

			"up\t<上传文件，参数为文件名>\n"

			"quit\t<退出系统，无需参数>\n"

	};

	SendRspns(tcps, &rspns);

 

	//循环获取客户端命令报文并进行处理

	for (;;) {

		CmdPacket cmd;

		if (!RecvCmd(tcps, (char *)&cmd))

			break;

		if (!ProcessCmd(tcps, &cmd, &clientaddr))

			break;

	}

 

	//线程结束前关闭控制连接套接字:

	closesocket(tcps);

	delete lpParam;

	return 0;

}

 

int main(int argc, char* argv[]) {

	SOCKET tcps_listen;  //FTP服务器控制连接侦听套接字

	struct threadData *pThInfo;

 

	if (!InitFTP(&tcps_listen))  //FTP初始化

		return 0;

	printf("FTP服务器开始监听，端口号为：%d。。。。。。\n", CMD_PORT);

 

	//循环接受客户端连接请求,并生成线程去处理:

	for (;;) {

		pThInfo = NULL;

		pThInfo = new threadData;

		if (pThInfo == NULL) {

			printf("为新线程申请空间失败。\n");

			continue;

		}

 

		int len = sizeof(struct threadData);

		//等待接受客户端控制连接请求

		pThInfo->tcps = accept(tcps_listen, (SOCKADDR*)&pThInfo->clientaddr, &len);

 

		//创建一个线程来处理相应客户端的请求:

		DWORD dwThreadId, dwThrdParam = 1;

		HANDLE hThread;

 

		hThread = CreateThread(

			NULL,               //无需安全性的继承

			0,					//默认线程栈大小

			ThreadFunc,			//线程入口函数

			pThInfo,			//线程入口函数的参数

			0,					//立即启动线程

			&dwThreadId);		//返回线程的id值

 

		//检查返回值是否创建线程成功

		if (hThread == NULL) {

			printf("创建线程失败。\n");

			closesocket(pThInfo->tcps);

			delete pThInfo;

		}

	}

 

	return 0;

 

}

 

//FTP初始化,创建一个侦听套接字:

 

int InitFTP(SOCKET *pListenSock) {

	//按照此步骤创建新的服务器端套接字，嗯，没错，前三个都是这个步骤

	//startup->socket->bind->listen

	WORD wVersionRequested;

	WSADATA wsaData;

	int err;

	SOCKET tcps_listen;

 

	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {

		printf("Winsock初始化时发生错误!\n");

		return 0;

	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {

		WSACleanup();

		printf("无效Winsock版本!\n");

		return 0;

	}

 

	tcps_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (tcps_listen == INVALID_SOCKET) {

		WSACleanup();

		printf("创建Socket失败!\n");

		return 0;

	}

 

	SOCKADDR_IN tcpaddr;

	tcpaddr.sin_family = AF_INET;

	tcpaddr.sin_port = htons(CMD_PORT);

	tcpaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	err = bind(tcps_listen, (SOCKADDR*)&tcpaddr, sizeof(tcpaddr));

	if (err != 0) {

		err = WSAGetLastError();

		WSACleanup();

		printf("Scoket绑定时发生错误!\n");

		return 0;

	}

	err = listen(tcps_listen, 3);

	if (err != 0) {

		WSACleanup();

		printf("Scoket监听时发生错误!\n");

		return 0;

	}

 

	*pListenSock = tcps_listen;

	return 1;

}

 

 

//建立数据连接

//pDatatcps:用于存储数据连接套接字

//pClientAddr:指向客户端的控制连接套接字地址,需要使用其中的IP地址

//返回值:0表示失败,1正常

int InitDataSocket(SOCKET *pDatatcps, SOCKADDR_IN *pClientAddr) {

	SOCKET datatcps;

 

	//创建socket

	datatcps = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (datatcps == INVALID_SOCKET) {

		printf("Creating data socket failed!\n");

		return 0;

	}

 

	SOCKADDR_IN tcpaddr;

	memcpy(&tcpaddr, pClientAddr, sizeof(SOCKADDR_IN));

	tcpaddr.sin_port = htons(DATA_PORT);    //如若有什么意外只需要在头文件修改端口值

 

	//请求连接客户端

	if (connect(datatcps, (SOCKADDR*)&tcpaddr, sizeof(tcpaddr)) == SOCKET_ERROR) {

		printf("Connecting to client failed!\n");

		closesocket(datatcps);

		return 0;

	}

 

	*pDatatcps = datatcps;

	return 1;

}

 

//处理命令报文

//tcps:控制连接套接字

//pcmd:指向待处理的命令报文

//pClientAddr:指向客户端控制连接套接字地址

//返回值:0表示有错或者需要结束连接,1正常

 

int ProcessCmd(SOCKET tcps, CmdPacket* pCmd, SOCKADDR_IN *pClientAddr) {

	SOCKET datatcps;   //数据连接套接字

	RspnsPacket rspns;  //回复报文

	FILE* file;

 

	//根据命令类型分派执行:

	switch (pCmd->cmdid) {

	case LS://展示当前目录下的文件列表

		//首先建立数据连接:

		if (!InitDataSocket(&datatcps, pClientAddr))

			return 0;

		//发送文件列表信息:

		if (!SendFileList(datatcps))

			return 0;

		break;

	case PWD://展示当前目录的绝对路径

		rspns.rspnsid = OK;

		//获取当前目录,并放至回复报文中

		if (!GetCurrentDirectory(RSPNS_TEXT_SIZE, rspns.text))

			strcpy(rspns.text, "Can't get current dir!\n");

		if (!SendRspns(tcps, &rspns))

			return 0;

		break;

	case CD://设置当前目录,使用win32 API 接口函数

		if (SetCurrentDirectory(pCmd->param)) {

			rspns.rspnsid = OK;

			if (!GetCurrentDirectory(RSPNS_TEXT_SIZE, rspns.text))

				strcpy(rspns.text, "切换当前目录成功！但是不能获取到当前的文件列表！\n");

		}

		else {

			strcpy(rspns.text, "不能更换到所选目录！\n");

		}

		if (!SendRspns(tcps, &rspns))   //发送回复报文

			return 0;

		break;

	case DOWN://处理下载文件请求:

		file = fopen(pCmd->param, "rb");   //打开要下载的文件

		if (file) {

			rspns.rspnsid = OK;

			sprintf(rspns.text, "下载文件%s\n", pCmd->param);

			if (!SendRspns(tcps, &rspns)) {

				fclose(file);

				return 0;

			}

			else {

				//创建额外的数据连接来传送数据:

				if (!InitDataSocket(&datatcps, pClientAddr)) {

					fclose(file);

					return 0;

				}

				if (!SendFile(datatcps, file))

					return 0;

				fclose(file);

			}

		}

		else  //打开文件失败

		{

			rspns.rspnsid = ERR;

			strcpy(rspns.text, "不能打开文件！\n");

			if (!SendRspns(tcps, &rspns))

				return 0;

		}

		break;

	case UP://处理上传文件请求

		//首先发送回复报文

		char filename[64];

		strcpy(filename, pCmd->param);

		//首先看一下服务器上是否已经有这个文件里，如果有就告诉客户端不用传输了

		if (FileExists(filename)) {

			rspns.rspnsid = ERR;

			sprintf(rspns.text, "服务器已经存在名字为%s的文件！\n", filename);

			if (!SendRspns(tcps, &rspns))

				return 0;

		}

		else {

			rspns.rspnsid = OK;

			if (!SendRspns(tcps, &rspns))

				return 0;

			//另建立一个数据连接来接受数据:

			if (!InitDataSocket(&datatcps, pClientAddr))

				return 0;

			if (!RecvFile(datatcps, filename))

				return 0;

		}

		break;

	case QUIT:

		printf("客户端断开连接。\n");

		rspns.rspnsid = OK;

		strcpy(rspns.text, "常来啊！\n");

		SendRspns(tcps, &rspns);

		return 0;

 

 

	}

 

	return 1;

 

}

 

//发送回复报文

int SendRspns(SOCKET tcps, RspnsPacket* prspns) {

	if (send(tcps, (char *)prspns, sizeof(RspnsPacket), 0) == SOCKET_ERROR) {

		printf("与客户端失去连接。\n");

		return 0;

	}

	return 1;

}

 

//接收命令报文

//tcps:控制连接套接字

//pCmd:用于存储返回的命令报文

//返回值:0表示有错或者连接已经断开,1表示正常

int RecvCmd(SOCKET tcps, char* pCmd) {					//used to receive command from client

	int nRet;

	int left = sizeof(CmdPacket);

 

	//从控制连接中读取数据,大小为 sizeof(CmdPacket):

	while (left) {

		nRet = recv(tcps, pCmd, left, 0);

		if (nRet == SOCKET_ERROR) {

			printf("从客户端接受命令时发生未知错误！\n");

			return 0;

		}

		if (!nRet) {

			printf("客户端关闭了连接！\n");

			return 0;

		}

 

		left -= nRet;

		pCmd += nRet;

	}

	return 1;   //成功获取命令报文

}

 

 

//发送一项文件信息:

int SendFileRecord(SOCKET datatcps, WIN32_FIND_DATA* pfd) {                    //used to send response to client

	char filerecord[MAX_PATH + 32];

	FILETIME ft;

	FileTimeToLocalFileTime(&pfd->ftLastWriteTime, &ft);

	SYSTEMTIME lastwtime;

	FileTimeToSystemTime(&ft, &lastwtime);

	char* dir = (char*)(pfd->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ? "<DIR>" : "");

	sprintf(filerecord, "%04d-%02d-%02d%02d:%02d   %5s   %10d   %-20s\n",

		lastwtime.wYear,

		lastwtime.wMonth,

		lastwtime.wDay,

		lastwtime.wHour,

		lastwtime.wMinute,

		dir,

		pfd->nFileSizeLow,

		pfd->cFileName);

	if (send(datatcps, filerecord, strlen(filerecord), 0) == SOCKET_ERROR) {

		printf("发送文件列表时发生未知错误！\n");

		return 0;

 

	}

	return 1;

}

 

 

//发送文件列表信息

//datatcps:数据连接套接字

//返回值:0表示出错,1表示正常

int SendFileList(SOCKET datatcps) {

	HANDLE hff;

	WIN32_FIND_DATA fd;

 

	//搜索文件

	hff = FindFirstFile("*", &fd);

	if (hff == INVALID_HANDLE_VALUE)  //发生错误

	{

		const char* errstr = "不能列出文件！\n";

		printf("文件列表输出失败！\n");

		if (send(datatcps, errstr, strlen(errstr), 0) == SOCKET_ERROR) {

			printf("发送给文件列表时发生未知错误！\n");

		}

		closesocket(datatcps);			return 0;

	}

 

	BOOL fMoreFiles = TRUE;

	while (fMoreFiles) {

		//发送此项文件信息:

		if (!SendFileRecord(datatcps, &fd)) {

			closesocket(datatcps);

			return 0;

		}

		//搜索下一个文件

		fMoreFiles = FindNextFile(hff, &fd);

	}

	closesocket(datatcps);

	return 1;

}

 

//通过数据连接发送文件

int SendFile(SOCKET datatcps, FILE* file) {

	char buf[1024];

	printf("发送文件数据中。。。。。。");

	for (;;) {				//从文件中循环读取数据并发送客户端

		int r = fread(buf, 1, 1024, file);

		if (send(datatcps, buf, r, 0) == SOCKET_ERROR) {

			printf("与客户端失去连接！\n");

			closesocket(datatcps);

			return 0;

		}

		if (r < 1024)   //文件传输结束

		{

			break;

		}

	}

	closesocket(datatcps);

	printf("完成传输!\n");

	return 1;

}

 

//接收文件

//datatcps:数据连接套接字,通过它来接收数据

//filename:用于存放数据的文件名

int RecvFile(SOCKET datatcps, char* filename) {

	char buf[1024];

	FILE* file = fopen(filename, "wb");

	if (!file) {

		printf("写入文件时发生未知错误！\n");

		fclose(file);

		closesocket(datatcps);

		return 0;

	}

	printf("接受文件数据中。。。。。。");

	while (1) {

		int r = recv(datatcps, buf, 1024, 0);

		if (r == SOCKET_ERROR) {

			printf("从客户端接受文件时发生未知错误！\n");

			fclose(file);

			closesocket(datatcps);

			return 0;

		}

		if (!r) {

			break;

		}

		fwrite(buf, 1, r, file);

	}

	fclose(file);

	closesocket(datatcps);

	printf("完成传输!\n");

	return 1;

}

 

//检测文件是否存在:

int FileExists(const char *filename)

{

	WIN32_FIND_DATA fd;

	if (FindFirstFile(filename, &fd) == INVALID_HANDLE_VALUE)

		return 0;

	return 1;

}

 

