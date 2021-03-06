// Server.cpp: 定义控制台应用程序的入口点。
//
//#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib,"ws2_32.lib")
#pragma warning( disable : 4996) 

int main(int argc, char* argv[])
{
	//初始化WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	//创建套接字
	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server == INVALID_SOCKET)
	{
		printf("socket error !");
		return 0;
	}

	//绑定IP和端口
	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8888);
	//serv_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	serv_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (bind(server, (LPSOCKADDR)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
	{
		printf("bind error !");
	}
	//读取服务器数据
	/*FILE * fp_hum = fopen("humidity.dat", "r");//打开dat格式的输入文件
	FILE * fp_light = fopen("light.dat", "r");//打开dat格式的输入文件
	FILE * fp_tem = fopen("temperature.dat", "r");//打开dat格式的输入文件
	int i = 0;
	int hum[30], light[30], tem[30];
	for (i = 0; i < 28; i++) {
		fscanf(fp_hum, "%d", &hum[i]);//从输入文件读取一个整数
		fscanf(fp_light, "%d", &light[i]);//从输入文件读取一个整数
		fscanf(fp_tem, "%d", &tem[i]);//从输入文件读取一个整数
	}
	fclose(fp_hum);
	fclose(fp_light);
	fclose(fp_tem);*/
	int hum[25] = {10,9,8,1,2,
							6,14,17,3,8,
							5,3,1,37,2,
							2,1,2,4,15,
							24,15,11,25,1};
	int light[25] = { 53,87,55,3,5,
							17,80,84,78,77,
							86,24,33,38,8,
							83,46,52,29,39,
							12,33,44,15,23};
	int tem[25] = { 32,37,4,40,27,
							25,36,2,16,12,
							19,31,6,15,28,
							1,39,21,14,10,
							8,17,3,33,29};
	//开始监听
	if (listen(server, 5) == SOCKET_ERROR)
	{
		printf("listen error !");
		return 0;
	}

	//循环接收数据
	SOCKET client;
	sockaddr_in ip_addr;
	int nAddrlen = sizeof(ip_addr);
	char revData[255];
	while (true)
	{
		while (true)
		{
			//连接
			printf("等待连接...\n");
			client = accept(server, (SOCKADDR *)&ip_addr, &nAddrlen);
			if (client == INVALID_SOCKET)
			{
				printf("accept error !");
				continue;
			}
			//连接成功
			printf("接受到一个连接\n");
			char * sendData = "连接成功！\n";
			send(client, sendData, strlen(sendData), 0);
			break;
		}
		while (true)
		{
			//接收数据
			int ret = recv(client, revData, 255, 0);
			if (ret > 0)
			{
				revData[ret] = 0x00;
				printf("查询时间：");
				printf(revData);
				printf("\n");
			}
			else if (ret == SOCKET_ERROR)
			{
				closesocket(client);
				break;
			}
			int time = atoi(revData);
			char str1[25] ;
			char str2[25] ;
			char str3[25] ;
			itoa(hum[time], str1, 10);
			itoa(light[time], str2, 10);
			itoa(tem[time], str3, 10);
			char str[100];
			strcpy(str, "当前传感器状态：湿度=");
			strcat(str, str1);
			strcat(str, "\t亮度=");
			strcat(str, str2);
			strcat(str, "\t温度=");
			strcat(str, str3);
			strcat(str, "\n");
			//发送数据
			char * sendData = str;
			send(client, sendData, strlen(sendData), 0);
		}
	}

	closesocket(server);
	WSACleanup();
	return 0;
}