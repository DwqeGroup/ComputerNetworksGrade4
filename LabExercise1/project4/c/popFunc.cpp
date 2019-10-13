#include "h.h"
#include "pop3.h"
//接收邮件类
POP3::POP3()
	{
		WSADATA WSAData;
		WSAStartup(MAKEWORD(2, 2), &WSAData);
		client = 0;
		MailCount = NULL;
	}
POP3::~POP3()
	{
	    Clear();
		WSACleanup();
	}
/*
1)LoginEMail连接邮箱函数
2)参数1：POP服务器地址      
  参数2：账号       
  参数3：密码    
  参数4：POP服务器监听端口(默认为110)

3)错误代码：-1：资源不足
            -2：域名解析有误
			-3：连接POP3服务器失败,检查你的域名以及端口号是否正确.
			-4：登录失败,检查网络
			-5：
			 0：登录成功 
*/
int POP3::LoginEMail(const char* pop_server,const char* UserName,const char* Password,USHORT port)
{  
		char Buffer[300];
		ZeroMemory(Buffer, 300);
		//创建套接字
		client = socket(AF_INET, SOCK_STREAM, 0);
		//设置阻塞超时
		int sec = 3000;
		setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char*)&sec, sizeof(int));
		//设置接收缓冲区大小
		sec = 5000;
		setsockopt(client, SOL_SOCKET, SO_RCVBUF, (char*)&sec, sizeof(int));
		if (client == INVALID_SOCKET)
			return -1;//资源不足
		//连接163的POP服务器
		SOCKADDR_IN dest;
		dest.sin_port = htons(port); //默认为110端口
		dest.sin_family = AF_INET;
		hostent* hptr = gethostbyname(pop_server);
		if (hptr == NULL)
		{
			Clear();
			return -2;//域名解析有误
		}
		memcpy(&dest.sin_addr.S_un.S_addr, hptr->h_addr_list[0], hptr->h_length);
		if (0 != connect(client, (SOCKADDR*)&dest, sizeof(SOCKADDR)))
		{
			Clear();
			return -3;//连接POP3服务器失败,检查你的域名以及端口号是否正确.
		}
		//输入账号
		sprintf_s(Buffer, 300, "USER %s\r\n", UserName);
		if (send(client, Buffer, strlen(Buffer), 0) <= 0)
		{
			Clear();
			return -4;//可能服务器强制中断了连接.
		}
		//查看响应
		if (!IsOk(Buffer, 300))
		{
			Clear();
			return -5;//登录时出错
		}
		//输入密码
		sprintf_s(Buffer, 300, "PASS %s\r\n", Password);
		if (send(client, Buffer, strlen(Buffer), 0) <= 0)
		{
			Clear();
			return -4;//可能服务器强制中断了连接.
		}
		//查看登录成功的响应
		if (!IsOk(Buffer, 300,1))
		{
			Clear();
			return -5;//登录时出错
		}
		

		//成功连接
		return 0;
}
//清理函数
void POP3::Clear()
	{
		//清除套接字资源
		if (client != INVALID_SOCKET && client != NULL)
		{
			closesocket(client);
			client = NULL;
		}
		if (MailCount)
		{
			delete[]MailCount;
			MailCount = NULL;
		}
	}
/*
1)IsOk检查POP服务器的响应字符串
2)TRUE为+OK
3)FALSE为-ERR
4)检查功能索引：
                1)默认为0,只单纯的检查是否为+OK
				2)1为输入密码后使用的,+OK登录成功后,会返回有几个邮件(Msg),总大小为多少字节信息.
*/
BOOL POP3::IsOk(char* Buffer,DWORD bufferlenth, DWORD Index)
{
	    char s[4] = {'+','O','K','\0'};
		ZeroMemory(Buffer, bufferlenth);
		if (recv(client, Buffer, bufferlenth, 0) <= 0)
		    return FALSE;
		if (0 != strncmp(s, Buffer, 3))
			return FALSE;
		//是否需要检查字段
		switch (Index)
		{
		case 1:
		      {
			      //解析邮件总数
			       if (!AnalyzePass(Buffer, bufferlenth))
				             return FALSE;
		       	   break;
		      }
		}

		return TRUE;
}
/*
邮件数量分析函数
用途：在输入密码后,解析封包,分析出邮件总个数.
*/
BOOL POP3::AnalyzePass(char* Buffer, DWORD bufferlenth)
{
	//跳过+OK字段
	char* str = Buffer + 4;
	//检查此封包是否为总个数封包
	//str[0]是否为数字字符串
	BOOL yes = FALSE; 
	for (char i = 48; i < 58; i++)
	{
		if (0 == strncmp(&str[0], &i, 1))
		{
			yes = TRUE;
			break;
		}
	}
	if (!yes)
	{   //清理封包,重新接收
		ZeroMemory(Buffer, bufferlenth);
		recv(client, Buffer, bufferlenth, 0);
	}
	for (char i = 48; i < 58; i++)
	{
		if (0 == strncmp(&str[0], &i, 1))
		{
			yes = TRUE;
			break;
		}
	}
	//还不是,直接返回FALSE
	if (!yes)
		return FALSE;
	/*邮件数量分析*/
	//找空格
	char* Space = strchr(str, ' ');
	//邮件数量字符串长度
	int Lenth = Space - str;
	//申请内存
	char* Number = new char[Lenth + 1];
	if (!Number)
		return FALSE;
	ZeroMemory(Number, Lenth + 1);
	memcpy(Number, str, Lenth);
	MailCount = Number;
	return TRUE;
}
//检查LIST命令封包
BOOL POP3::AnalyzeList(char* Buffer, DWORD bufferlenth,char* Index,int* Size)
{
	//记录长度
	char lenthstr[200];
	ZeroMemory(lenthstr, 200);
	//跳过+OK字段
	char* str = Buffer + 4 + strlen(Index) + 1;
	//找换行
	char* Space = strchr(str, '\r');
	//长度字符串的长度
	int Lenth = Space - str;
	memcpy(lenthstr, str, Lenth);
	*Size = atoi(str);
	return TRUE;
}
//查看指定邮件，返回一个堆内存没处理
char* POP3::Look(char* Index)
{
	//邮件的长度
	int Size;
	//邮件内容
	char* Mailstr = NULL;
	//命令
	char cmd[200];
	ZeroMemory(cmd, 200);
	//发送RETR接收2个数据：1、长度（返回的长度中不包括 ".\r\n"），2、邮件数据
	sprintf_s(cmd, 200, "RETR %s\r\n", Index);
	send(client, cmd, strlen(cmd), 0);
	ZeroMemory(cmd, 200);
	if (recv(client, cmd, 200, 0) <= 0)
		return NULL;
	//解析长度字段
	AnalyzeRetr1(cmd, 200, &Size);
	if (Size <= 0)
		return NULL;
	Size += 3; //".\r\n"包括
	int Bufferlenth = Size + 1;
	Mailstr = new char[Bufferlenth];
	if (!Mailstr)
		return NULL;
	ZeroMemory(Mailstr, Bufferlenth);
	int recv_lenth = recv(client, Mailstr, Size, 0);
	if (recv_lenth <= 0)
	{
		delete[]Mailstr;
		return NULL;
	}
	int Ret = 0;
	while(recv_lenth < Size)
	{
	    Ret = recv(client, Mailstr + recv_lenth, Size - recv_lenth, 0);
		if (Ret <= 0)
		{
			delete[]Mailstr;
			return NULL;
		}
		recv_lenth  += Ret;
	}
	//下载完成,解析RETR封包
	if (!AnalyzeRetr2(Mailstr, Bufferlenth))
	{
		delete[] Mailstr;
		return NULL;
	}
	//返回记录封包信息
	return Mailstr;
}
//返回邮件大小,-1位错误
int POP3::GetMailSize(char* Index)
{
	//邮件的长度
	int Size;
	//LIST命令
	char cmd[200];
	ZeroMemory(cmd, 200);
	sprintf_s(cmd, 200, "LIST %s\r\n", Index);
	send(client, cmd, strlen(cmd), 0);
	ZeroMemory(cmd, 200);
	if (recv(client, cmd, 200, 0) <= 0)
		return -1;
	if (!AnalyzeList(cmd, 200, Index, &Size))
		return -1;
	return Size;
}
//返回邮件的唯一标识符
char* POP3::GetID(char* Index)
{
	char* p = new char[200];
	if (!p)
		return NULL;
	ZeroMemory(p, 200);
	sprintf_s(p, 200, "UIDL %s\r\n", Index);
	send(client, p, strlen(p), 0);
	ZeroMemory(p, 200);
	if (recv(client, p, 200, 0) <= 0)
		return NULL;
	char* s = StrStr(p, Index);
	s += 1;
	char* p1 = new char[strlen(s) + 1];
	if (!p1)
	{
		delete[]p;
		return NULL;
	}
	char* p2 = StrStr(s,"\r\n");
	ZeroMemory(p1, strlen(s) + 1);
	memcpy(p1, s, p2 - s);
	delete[]p;
	return p1;
}
//将邮件标记为DELE删除,可以RSET取消,QUIT更新就删除了
BOOL POP3::DeleteMail(char* Index)
{
	//DELE命令
	char cmd[200];
	ZeroMemory(cmd, 200);
	sprintf_s(cmd, 200, "DELE %s\r\n", Index);
	send(client, cmd, strlen(cmd), 0);
	return IsOk(cmd, 200);
}
//取消所有删除标记
BOOL POP3::CancelDeleteMail()
{
	//DELE命令
	char cmd[200];
	ZeroMemory(cmd, 200);
	sprintf_s(cmd, 200, "RSET\r\n");
	send(client, cmd, strlen(cmd), 0);
	return IsOk(cmd, 200);
}
//立即删除标记为删除的邮件
BOOL POP3::MailUpdate()
{
	//QUIT命令
	char cmd[200];
	ZeroMemory(cmd, 200);
	sprintf_s(cmd, 200, "QUIT\r\n");
	send(client, cmd, strlen(cmd), 0);
	return IsOk(cmd, 200);
}
//获取邮件总个数  -1为失败
int POP3::GetMailCount()
{
	//STAT命令
	char cmd[200];
	ZeroMemory(cmd, 200);
	sprintf_s(cmd, 200, "STAT\r\n");
	send(client, cmd, strlen(cmd), 0);
	ZeroMemory(cmd, 200);
	if (recv(client, cmd, 200, 0) <= 0)
		return -1;

	//跳过+OK字段
	char* str = cmd + 4;

	//找空格
	char* Space = strchr(str, ' ');
	//邮件数量字符串长度
	int Lenth = Space - str;
	//申请内存
	char* Number = new char[Lenth + 1];
	if (!Number)
		return FALSE;
	ZeroMemory(Number, Lenth + 1);
	memcpy(Number, str, Lenth);
	int Count = atoi(Number);
	if (Count < 0)
		return -1;
	delete[] Number;
	return Count;
}
//解析RETR封包一
void POP3::AnalyzeRetr1(char* Buffer, DWORD bufferlenth,int* Size)
{
	//记录长度
	char lenthstr[200];
	ZeroMemory(lenthstr, 200);
	//跳过+OK字段
	char* str = Buffer + 4;
	//找换行
	char* Space = strchr(str, ' ');
	//长度字符串的长度
	int Lenth = Space - str;
	memcpy(lenthstr, str, Lenth);
	*Size = atoi(str);
}
//解析RETR封包二
BOOL POP3::AnalyzeRetr2(char* Buffer, DWORD bufferlenth)
{
	//最初发送者主机名以及它的IP地址
	char* HostName = new char[MAX_PATH];
	if (!HostName)
		return FALSE;
	ZeroMemory(HostName, MAX_PATH);
	//From:字段
	char* From = new char[MAX_PATH];
	if (!From)
		return FALSE;
	ZeroMemory(From, MAX_PATH);
	//To:字段
	char* To = new char[MAX_PATH];
	if (!To)
		return FALSE;
	ZeroMemory(To, MAX_PATH);
	//Subject:字段
	char* Subject = new char[MAX_PATH];
	if (!Subject)
		return FALSE;
	ZeroMemory(Subject, MAX_PATH);
	//Date:字段
	char* Date = new char[MAX_PATH];
	if (!Date)
		return FALSE;
	ZeroMemory(Date, MAX_PATH);

	//解析最初发送主机和它的IP地址
	mail_ip(Buffer, HostName, MAX_PATH);
	//处理From:字段
	mail_from(Buffer, From, MAX_PATH);
	//处理To:字段
	mail_to(Buffer, To, MAX_PATH);
	//处理Subject:字段
	mail_subject(Buffer, Subject, MAX_PATH);
	//处理Date:字段
	mail_date(Buffer, Date, MAX_PATH);
	//处理内容
	char* content = mail_content(Buffer, bufferlenth);
	//格式化缓冲区
	ZeroMemory(Buffer, bufferlenth);
	sprintf_s(Buffer, bufferlenth, "发信人:%s\r\n收信人:%s\r\n主题:%s\r\n日期:%s\r\n\r\n内容:\r\n%s\r\n\r\n发信主机名:%s\r\n"
		, From, To, Subject, Date, content, HostName);
	delete[]HostName;
	delete[]From;
	delete[]To;
	delete[]Subject;
	delete[]Date;
	delete[]content;
	return TRUE;
}
//解析主机名和IP地址
void POP3::mail_ip(char* Buffer, char* HostName, DWORD HostName_Buffer_lenth)
{
	int lenth = strlen("From:");//域的长度
	char* pstr = NULL;
	char* p = StrStr(Buffer, "from ");
	while (p != NULL)
	{
		pstr = p + lenth;
		p = StrStr(p + lenth, "from ");
	}
	if (pstr != NULL)
	{
		//找换行
		char* Space = strchr(pstr, '\r');
		int lenth = Space - pstr;
		memcpy_s(HostName, HostName_Buffer_lenth, pstr, lenth);
	}
}
//解析说明发信人字段
void POP3::mail_from(char* Buffer, char* UserName, DWORD UserName_Buffer_lenth)
{
	//查找From:字段
	int lenth = strlen("From:");//域的长度
	char* pstr = NULL;
	char* p = StrStr(Buffer, "From:");
	while (p != NULL)
	{
		pstr = p + lenth;
		p = StrStr(p + lenth, "From:");
	}
	//如果发现发信人姓名为Base64编码
	char* pBase64 = StrStr(Buffer, "B?");
	if (pBase64)
	{
		char y_sss;
		char* sss = NULL;
		BOOL utf8 = FALSE;
		pBase64 += strlen("B?");
		sss = StrStr(Buffer, "=?UTF-8?B");
		if (sss)
			utf8 = TRUE;
		sss = StrStr(pBase64, "?=");
		if (sss)
		{
			sss += 2;
			y_sss = sss[0];
			sss[0] = '\0';
		}
		//解析出名称
		char* s = base64_decode(pBase64);
		//恢复刚才修改的字节
		if (sss)
			sss[0] = y_sss;
		//如果为UTF-8字符集
		char* utf = NULL;
		if (utf8)
			utf = Utf8ToAscii(s);
		//解析地址
		sss = StrStr(pBase64, "<");
		char* zzz = NULL;
		char* MailAddress = NULL;
		int Adrresslenth;
		if (sss)
		{
			zzz = StrStr(sss, ">");
			zzz += 1;
			Adrresslenth = zzz - sss;
			MailAddress = new char[Adrresslenth + 1];
			if (!MailAddress)
				exit(0);
			ZeroMemory(MailAddress, Adrresslenth + 1);
			memcpy(MailAddress, sss, Adrresslenth);
		}
		ZeroMemory(UserName, UserName_Buffer_lenth);
		if (utf8)
		{
			memcpy(UserName, utf, strlen(utf));
			delete[] utf;
		}
		else
		{
			memcpy(UserName, s, strlen(s));
		}
		if (sss)
		{
			sss = UserName + strlen(s);
			sss[0] = ' ';
			sss += 1;
			memcpy(sss, MailAddress, Adrresslenth);
			delete[] MailAddress;
		}
		delete[]s;
		return;
	}
	//获取发信人账号
	char* UserStart = strchr(pstr, '<');
	char* UserEnd = strchr(pstr, '\r');
	int Userlenth = UserEnd - UserStart;
	memcpy_s(UserName, UserName_Buffer_lenth, UserStart, Userlenth);
}
//解析说明收信人字段
void POP3::mail_to(char* Buffer, char* UserName, DWORD UserName_Buffer_lenth)
{
	
	//查找to:字段
	int lenth = strlen("To:");//域的长度
	char* pstr = NULL;
	char* p = StrStr(Buffer, "To:");
	while (p != NULL)
	{
		pstr = p + lenth;
		p = StrStr(p + lenth, "To:");
	}
	//获取收信人账号
	char* UserEnd = strchr(pstr, '\r');
	int Userlenth = UserEnd - pstr;
	memcpy_s(UserName, UserName_Buffer_lenth, pstr, Userlenth);

	//如果发现收信人姓名为Base64编码
	char* pBase64 = StrStr(UserName, "B?");
	if (pBase64)
	{
		char y_sss;
		char* sss = NULL;
		pBase64 += strlen("B?");
		sss = StrStr(UserName, "?=");
		if (sss)
		{
			sss += 2;
			y_sss = sss[0];
			sss[0] = '\0';
		}
		//解析出名称
		char* s = base64_decode(pBase64);
		//恢复刚才修改的字节
		if (sss)
			sss[0] = y_sss;
		//解析地址
		sss = StrStr(UserName, "<");
		char* zzz = NULL;
		char* MailAddress = NULL;
		int Adrresslenth;
		if (sss)
		{
			zzz = StrStr(UserName, ">");
			zzz += 1;
			Adrresslenth = zzz - sss;
			MailAddress = new char[Adrresslenth + 1];
			if (!MailAddress)
				exit(0);
			ZeroMemory(MailAddress, Adrresslenth + 1);
			memcpy(MailAddress, sss, Adrresslenth);
		}
		ZeroMemory(UserName, UserName_Buffer_lenth);
		memcpy(UserName, s, strlen(s));
		if (sss)
		{
			sss = UserName + strlen(s);
			sss[0] = ' ';
			sss += 1;
			memcpy(sss, MailAddress, Adrresslenth);
			delete[] MailAddress;
		}
		delete[]s;

	}
	

}
//解析邮件主题
void POP3::mail_subject(char* Buffer, char* Subject, DWORD Subject_Buffer_lenth)
{
	//查找Subject:字段
	int lenth = strlen("Subject:");//域的长度
	char* pstr = NULL;
	char* p = StrStr(Buffer, "Subject:");
	while (p != NULL)
	{
		pstr = p + lenth;
		p = StrStr(p + lenth, "Subject:");
	}
	char* SubjectEnd = strchr(pstr, '\r');
	int Subjectlenth = SubjectEnd - pstr;
	memcpy_s(Subject, Subject_Buffer_lenth, pstr, Subjectlenth);
    //如果为Base64编码,那么解密
	char* pBase64 = StrStr(Subject, "B?");
	if (pBase64)
	{
		pBase64 += strlen("B?");
		char* s = base64_decode(pBase64);
		//还要查看一下是不是UTF-8编码的,如果是,还要转换成普通字符串显示
		pBase64 = StrStr(Subject, "UTF-8");
		ZeroMemory(Subject, Subject_Buffer_lenth);
		char* utf = NULL;
		//为UTF8编码
		if (pBase64)
		{
			 utf = Utf8ToAscii(s);
			 memcpy(Subject, utf, strlen(utf));
			 delete[]utf;
		}
		else
		{
			memcpy(Subject, s, strlen(s));
		}
		delete[]s;
	}
}
//解析日期
void POP3::mail_date(char* Buffer, char* Date, DWORD Date_Buffer_lenth)
{
	//查找Subject:字段
	int lenth = strlen("Date:");//域的长度
	char* pstr = NULL;
	char* p = StrStr(Buffer, "Date:");
	while (p != NULL)
	{
		pstr = p + lenth;
		p = StrStr(p + lenth, "Date:");
	}
	char* DateEnd = strchr(pstr, '\r');
	int Datelenth = DateEnd - pstr;
	memcpy_s(Date, Date_Buffer_lenth, pstr, Datelenth);

}
//解析内容
char* POP3::mail_content(char* Buffer, DWORD Buffer_Lenth)
{
	//Base64类型①
	char* base = StrStr(Buffer, "base64");
	char* base_str = NULL; //Base64转换后字符串
	if (base)
	{
		//将整个被加密的数据放入pstr指向的内存中
		base += strlen("base64") + strlen("\r\n\r\n");
		char* end = StrStr(base, "\r\n\r\n");
		end += 2;
		int basestrlenth = end - base;
		char* pstr = new char[basestrlenth + 1];
		if (!pstr)
			exit(0);
		ZeroMemory(pstr, basestrlenth + 1);
		memcpy_s(pstr, basestrlenth, base, basestrlenth);
		//遍历开头和\r\n行,将它们转换普通字符串
		char* pp = pstr;
		end = pp;
		while (TRUE)
		{			
			end = StrStr(pp, "\r\n");
			if (end == NULL)
				break;
			end[0] = '\0';
			char* z = base64_decode(pp);
			if (!base_str)
			{//第一次
				int zlenth = strlen(z);
				base_str = new char[zlenth + 1];
				if (!base_str)
					exit(0);
				ZeroMemory(base_str, zlenth + 1);
				memcpy(base_str, z, zlenth);
			}
			else
			{//下一行
				int str_lenth = strlen(base_str);
				int zlenth = strlen(z);
				char* mm = new char[str_lenth + zlenth  + 1];
				if (!mm)
					exit(0);
				ZeroMemory(mm, str_lenth + zlenth + 1);
				memcpy(mm, base_str, str_lenth);
				delete[] base_str;
				//处理新行
				char* aa = mm + str_lenth;
				memcpy(aa, z, zlenth);
				base_str = mm;
			}
			//释放内存
			delete[]z;
			z = NULL;
			//pp到下一行
			pp = end + 2;
		}
		//释放内存
		delete[]pstr;
		//返回字符串
		return base_str;
	}

	//普通类型②

	//查找\r\n\r\n字段
	int lenth = strlen("\r\n\r\n"); //域的长度
	char* pstr = NULL;
	char* p = StrStr(Buffer, "\r\n\r\n");
	while (p != NULL)
	{
		pstr = p + lenth;
		p = StrStr(p + lenth, "\r\n\r\n");
	}
	//查找结尾
	p = StrStr(pstr, "\r\n\r\n");
	if (!p)
	{
		char* l = NULL;
		l = StrStr(pstr, "\r\n");
		while (l != NULL)
		{
			p = l;
			l = StrStr(l + strlen("\r\n"), "\r\n");
		}
	}
	lenth = p - pstr;
	if (lenth <= 0)
		return NULL;
	char* Data = new char[lenth + 1];
	if (!Data)
		return NULL;
	ZeroMemory(Data, lenth + 1);
	memcpy_s(Data, lenth, pstr, lenth);
	return Data;
}
char* POP3::Look(char* Index, int n)
{
	if (n <= 0)
		return NULL;
	//邮件的长度
	int Size;
	//邮件内容
	char* Mailstr = NULL;
	//命令
	char cmd[200];
	ZeroMemory(cmd, 200);
	//先返回总长度,再返回前几行的内容
	sprintf_s(cmd, 200, "TOP %s %d\r\n", Index,n);
	send(client, cmd, strlen(cmd), 0);
	ZeroMemory(cmd, 200);
	if (recv(client, cmd, 200, 0) <= 0)
		return NULL;
	//解析长度字段
	AnalyzeRetr1(cmd, 200, &Size);
	if (Size <= 0)
		return NULL;
	Size += 3; //".\r\n"包括
	int Bufferlenth = Size + 1;
	Mailstr = new char[Bufferlenth];
	if (!Mailstr)
		return NULL;
	ZeroMemory(Mailstr, Bufferlenth);
	if (recv(client, Mailstr, Size, 0) <= 0)
	{
		delete[]Mailstr;
		return NULL;
	}
	//下载完成,解析RETR封包
	if (!AnalyzeRetr2(Mailstr, Bufferlenth))
	{
		delete[] Mailstr;
		return NULL;
	}
	//返回记录封包信息
	return Mailstr;
}