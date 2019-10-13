#pragma once
//接收邮件类
class POP3
{
public:
	//加载与卸载Winsock库
	POP3();
	~POP3();
public:
	//连接邮箱
	int LoginEMail(const char* pop_server, const char* UserName, const char* Password, USHORT port = 110);
	//查看指定邮件
	char* Look(char* Index);
	//查看指定邮件,只显示前n行
	char* Look(char* Index,int n);
	//返回指定邮件的大小（不包括“.\r\n”）
	int GetMailSize(char* Index);
	//返回邮件的唯一标识符
	char* GetID(char* Index);
	//标记删除邮件（退出或QUIT删除,RSET标记取消）
	BOOL DeleteMail(char* Index);
	//取消所有删除邮件标记
	BOOL CancelDeleteMail();
	//立即删除标记为删除的邮件
	BOOL MailUpdate();
	//返回邮件总个数
	int GetMailCount();
	//清理函数
	void Clear();
private:
	//检查响应
	BOOL IsOk(char* Buffer, DWORD bufferlenth,DWORD Index = 0);
private:
	//解析PASS封包
	BOOL AnalyzePass(char* Buffer, DWORD bufferlenth);
	//解析LIST封包
	BOOL AnalyzeList(char* Buffer, DWORD bufferlenth, char* Index, int* Size);
	//解析RETR封包1
	void AnalyzeRetr1(char* Buffer, DWORD bufferlenth, int* Size);
	//解析RETR封包2
	BOOL AnalyzeRetr2(char* Buffer, DWORD bufferlenth);
private://用于解析各个邮件内容字段的函数集合
	void mail_ip(char* Buffer, char* HostName,DWORD HostName_Buffer_lenth);
	void mail_from(char* Buffer, char* UserName, DWORD UserName_Buffer_lenth);
	void mail_to(char* Buffer, char* UserName, DWORD UserName_Buffer_lenth);
	void mail_subject(char* Buffer, char* Subject, DWORD Subject_Buffer_lenth);
	void mail_date(char* Buffer, char* Date, DWORD Date_Buffer_lenth);
	char* mail_content(char* Buffer,DWORD Buffer_Lenth);
private:
	SOCKET client;  //套接字
public:
	char* MailCount;//邮件的总数量字符串,登录后由new申请的内存
};