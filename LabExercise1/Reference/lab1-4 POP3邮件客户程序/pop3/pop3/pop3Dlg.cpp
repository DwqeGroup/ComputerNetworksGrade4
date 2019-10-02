
// pop3Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "pop3.h"
#include "pop3Dlg.h"
#include "afxdialogex.h"
#include "pop_socket.h"
#include "base64.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Cpop3Dlg 对话框




Cpop3Dlg::Cpop3Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cpop3Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cpop3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, cedit_popserver);
	DDX_Control(pDX, IDC_EDIT3, cedit_user);
	DDX_Control(pDX, IDC_EDIT2, cedit_psw);
	DDX_Control(pDX, IDC_LIST1, clistbox_mail);
	DDX_Control(pDX, IDC_EDIT4, cedit_cont);
	DDX_Control(pDX, IDC_BUTTON2, cbutton_open);
	DDX_Control(pDX, IDC_BUTTON3, cbutton_dele);
	DDX_Control(pDX, IDC_BUTTON4, cbutton_fresh);
}

BEGIN_MESSAGE_MAP(Cpop3Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &Cpop3Dlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &Cpop3Dlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &Cpop3Dlg::OnBnClickedButton3)
	ON_LBN_SELCHANGE(IDC_LIST1, &Cpop3Dlg::OnLbnSelchangeList1)
	ON_BN_CLICKED(IDCANCEL, &Cpop3Dlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON4, &Cpop3Dlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &Cpop3Dlg::OnBnClickedButton5)
END_MESSAGE_MAP()


// Cpop3Dlg 消息处理程序

BOOL Cpop3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	POP3.Initialize();
	cedit_popserver.SetWindowTextA("pop.163.com");
	cedit_user.SetWindowTextA("vergil_1");
	cedit_psw.SetWindowTextA("8895689");
	cbutton_open.EnableWindow(false);
	cbutton_dele.EnableWindow(false);
	cbutton_fresh.EnableWindow(false);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void Cpop3Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR Cpop3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void Cpop3Dlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	string server,user,psw;
	char temp[200];
	cedit_popserver.GetWindowTextA(temp,200);
	server=string(temp);
	cedit_user.GetWindowTextA(temp,200);
	user=string(temp);
	cedit_psw.GetWindowTextA(temp,200);
	psw=string(temp);

	POP3.login(server,user,psw);
	POP3.listmail();

	cbutton_open.EnableWindow(false);
	cbutton_dele.EnableWindow(false);
	cbutton_fresh.EnableWindow(true);

	MessageBoxA("登录成功","提示信息：",MB_DEFBUTTON1 | MB_ICONEXCLAMATION);
	string cont=string(buffer);
	int index;
	do{
		index=cont.find("\r\n");
		if(index==-1)
			break;
		if(cont[0]!='.' && cont[0]!='+')
			clistbox_mail.AddString(cont.substr(0,index).c_str());
		index+=2;
		cont=cont.substr(index);
	}while(1);
}


void Cpop3Dlg::OnBnClickedButton2()
{
	//打开
	int num=clistbox_mail.GetCurSel();
	CString str;
	clistbox_mail.GetText(num,str);
	string s(str.GetBuffer());
	string snum=s.substr(0,s.find(' '));
	num=atoi(snum.c_str());
	string recv=POP3.retr(num);
	
/*	int f = recv.find("From:",500);
	int t = recv.find("To:",500);
	int sub = recv.find("Subject:",500);
	int m = recv.find("Mime-Version:",500);
	int c1 = recv.find("------=",1000);
	int c2 = recv.find("------=",c1+1);

	string from = recv.substr(f, t-f);
	string to = recv.substr(t, sub-t);
	string subject = recv.substr(sub, m-sub);
	string text = recv.substr(c1,c2-c1);

	from.replace(5,from.find("<")-5,"");
	to.replace(3,to.find("<")-3,"");
	text.replace(0,text.find("base64")+6,"");

	string cont = from+"\n"+to+"\n"+subject+"\n"+text;*/
	cedit_cont.SetWindowTextA(recv.c_str());
}


void Cpop3Dlg::OnBnClickedButton3()
{
	//删除
	int num=clistbox_mail.GetCurSel();
	CString str;
	clistbox_mail.GetText(num,str);
	string s(str.GetBuffer());
	string snum=s.substr(0,s.find(' '));
	num=atoi(snum.c_str());
	POP3.dele(num);
	cedit_cont.SetWindowTextA(buffer);
}


void Cpop3Dlg::OnLbnSelchangeList1()
{
	//选择邮件
	cbutton_open.EnableWindow(true);
	cbutton_dele.EnableWindow(true);
}


void Cpop3Dlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	POP3.quit();
	CDialogEx::OnCancel();
}


void Cpop3Dlg::OnBnClickedButton4()
{
	//刷新
	cbutton_open.EnableWindow(false);
	cbutton_dele.EnableWindow(false);

	clistbox_mail.ResetContent();
	cedit_cont.SetWindowTextA("");
	POP3.listmail();
	string cont=string(buffer);
	int index;
	do{
		index=cont.find("\r\n");
		if(index==-1)
			break;
		if(cont[0]!='.' && cont[0]!='+')
			clistbox_mail.AddString(cont.substr(0,index).c_str());
		index+=2;
		cont=cont.substr(index);
	}while(1);	
}


void Cpop3Dlg::OnBnClickedButton5()
{
	// 登出
	POP3.quit();
	cbutton_open.EnableWindow(false);
	cbutton_dele.EnableWindow(false);
	cbutton_fresh.EnableWindow(false);
	cedit_cont.SetWindowTextA("");
	clistbox_mail.ResetContent();
	cedit_popserver.SetWindowTextA("pop.163.com");
}
