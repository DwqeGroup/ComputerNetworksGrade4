#pragma once
#include <winsock2.h>
#include <string>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;
extern char buffer[10000];
class POP_SOCKET
{
public:
	POP_SOCKET(void){};
	~POP_SOCKET(void){};
	BOOL Initialize(void);	
	SOCKET m_socket;
	string cmd;
	int listmail();
	string retr(int num);
	int dele(int num);
	int stat(int num);
	int quit();
	int login(string servername,string username,string psw);

};