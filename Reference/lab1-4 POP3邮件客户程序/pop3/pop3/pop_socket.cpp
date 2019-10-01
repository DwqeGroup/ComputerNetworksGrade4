#include "stdafx.h"
#include "pop_socket.h"

void clearAll(SOCKET s) {
	if (s != INVALID_SOCKET)
		closesocket(s);
	WSACleanup();
}

BOOL POP_SOCKET::Initialize()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 1, 1 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		return FALSE;
	}


	if ( LOBYTE( wsaData.wVersion ) != 1 ||
		HIBYTE( wsaData.wVersion ) != 1 ) 
	{
		WSACleanup();
		return FALSE; 
	}

	return TRUE;
}

int POP_SOCKET::listmail()
{
	cmd="list\r\n";
	send(m_socket, cmd.c_str(), cmd.length(), 0);
	Sleep(1000);
	int len = recv(m_socket, buffer, 9999, 0);
	buffer[len]='\0';
	return len;
}
string POP_SOCKET::retr(int num)
{
	cmd="retr ";
	char s_num[20];
	_itoa_s(num,s_num,10);
	cmd+=string(s_num);
	cmd+="\r\n";
	send(m_socket, cmd.c_str(), cmd.length(), 0);
	Sleep(1000);
	string context="";
	int len=5000;
	while(len==5000)
	{
		len= recv(m_socket, buffer, 5000, 0);
		buffer[len] = '\0';
		context+=string(buffer);

	}

	return context;
}
int POP_SOCKET::dele(int num)
{
	cmd="dele ";
	char s_num[20];
	_itoa_s(num,s_num,10);
	cmd+=string(s_num);
	cmd+="\r\n";
	send(m_socket, cmd.c_str(), cmd.length(), 0);
	Sleep(1000);
	int len = recv(m_socket, buffer, 5000, 0);
	buffer[len]='\0';
	return len;
}
int POP_SOCKET::stat(int num)
{
	cmd="stat\r\n";
	send(m_socket, cmd.c_str(), cmd.length(), 0);
	Sleep(1000);
	int len = recv(m_socket, buffer, 5000, 0);
	buffer[len]='\0';
	return len;
}
int POP_SOCKET::quit()
{
	cmd="quit\r\n";
	send(m_socket, cmd.c_str(), cmd.length(), 0);
	Sleep(1000);
	int len = recv(m_socket, buffer, 5000, 0);
	buffer[len]='\0';


	closesocket(m_socket);
	WSACleanup();
	return len;
}
int POP_SOCKET::login(string servername,string username,string psw)
{
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(1,1), &wsadata) != 0) {
		printf("*WinSock error!\n");
		exit(0);
	}

	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(110);
	struct hostent* hptr = gethostbyname(servername.c_str());
	if (hptr == NULL) {
		printf("*The host name is Invalid!\n");
		clearAll(m_socket);
		exit(0);
	}
	memcpy(&sin.sin_addr.S_un.S_addr, hptr->h_addr_list[0], hptr->h_length);
	printf("IP of %s is : %d:%d:%d:%d\n",servername.c_str(),
		sin.sin_addr.S_un.S_un_b.s_b1,
		sin.sin_addr.S_un.S_un_b.s_b2,
		sin.sin_addr.S_un.S_un_b.s_b3,
		sin.sin_addr.S_un.S_un_b.s_b4);

	//connection
	printf("Connect to %s...\n", inet_ntoa(sin.sin_addr));
	if (connect(m_socket, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)	{
		printf("*connect failed!\n");
		clearAll(m_socket);
		exit(0);
	} else {
		printf("Connect success...\n");
	}
	char buffer[1024];
	int len = recv(m_socket, buffer, 1024, 0);
	if(buffer[0] == '+') {
		buffer[len] = '\0';
		printf("\n%s\n", buffer);
	} else {
		printf("*connect failed!\n");
		clearAll(m_socket);
		exit(0);
	}
	// login
	char tmp[50];
	username="USER "+username+"\r\n";

	send(m_socket, username.c_str(),username.length(), 0);
	len = recv(m_socket, buffer, 1024, 0);
	if (buffer[0] == '+') {
		buffer[len] = '\0';
		printf("%s\n", buffer);
	}
	string password = "PASS "+psw+"\r\n";

	send(m_socket, password.c_str(), password.length(), 0);
	len = recv(m_socket, buffer, 1024, 0);
	if (buffer[0] == '+') {
		buffer[len] = '\0';
		printf("%s\n", buffer);
	} else {
		printf("*login error!\n");
		clearAll(m_socket);
		exit(0);
	}

	return 1;
}