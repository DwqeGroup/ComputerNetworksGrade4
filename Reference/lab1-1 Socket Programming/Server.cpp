#include <tchar.h>
#include <SDKDDKVer.h>
#include <WINSOCK2.H>
#include <STDIO.H>

#pragma  comment(lib,"ws2_32.lib")


int main(int argc, char* argv[])
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}

	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		printf("invalid socket !");
		return 0;
	}

	sockaddr_in serAddr;
	char server_addr[25];
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8888);
	//serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.2");
	printf("请指定服务器地址：");
	scanf("%s", server_addr);
	serAddr.sin_addr.S_un.S_addr = inet_addr(server_addr);
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("connect error !");
		closesocket(sclient);
		return 0;
	}

	char sendData[100];
	char recData[255];
	int ret = recv(sclient, recData, 255, 0);
	if (ret > 0)
	{
		recData[ret] = 0;
		printf(recData);		//接受成功的提示
	}
	while (true)
	{
		scanf("%s", sendData);
		send(sclient, sendData, strlen(sendData), 0);

		char recData[255];
		int ret = recv(sclient, recData, 255, 0);
		if (ret > 0)
		{
			recData[ret] = 0x00;
			printf(recData);
		}
	}


	closesocket(sclient);
	WSACleanup();
	return 0;
}
