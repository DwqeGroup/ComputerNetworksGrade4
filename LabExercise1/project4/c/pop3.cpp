#include "h.h"
#include "pop3.h"
/*
使用说明：
1)实现了大多数命令(除了无用的以外)
2)解析Base64编码,UTF-8编码
3)不能登录QQ邮箱,因为与使用QQ的POP服务器要开启SSL加密传输协议,这里使用的是普通POP协议.
4)测试：建议163邮箱（可能有些邮件解析出来之后有乱码,这就是没解析好,因为每一个邮件的编码和格式都不同）
5)如果遇到乱码,可以按着这里的解析的思路去解析那种编码就好了.
*/
int main()
{
	system("color 4e");
	POP3 pop3;

	
	//登录163邮箱
	if (!pop3.LoginEMail("pop.163.com", "zqydwqe@163.com", "zqyzqy123456"))
		printf("登录成功..\r\n\r\n邮件总数:%s\r\n\r\n", pop3.MailCount);
	else
		printf("登录失败..\r\n\r\n");

	//显示全部邮件
	int Index = 1;
	int All = atoi(pop3.MailCount);
	char str_Index[100]; //转换成数字字符串
	//显示所有邮件
	while (Index <= 1)
	{
		itoa(Index, str_Index, 10);
		char* str = pop3.Look(str_Index);
		if (str)
		{
			printf("-----------------------------------------------------\r\n");
			char* id = pop3.GetID(str_Index);
			printf("打开邮件成功,您正在查看第%s封邮件<大小为:%d字节>\r\n邮件ID:%s.\r\n\r\n", str_Index, pop3.GetMailSize(str_Index),id);
			cout << str << endl;
			delete[] str;
			delete[] id;
		}
		else
		{
			printf("打开邮件失败,请重试.\r\n\r\n");
		}
		Index++;
	}
	getchar();
	return 0;
}