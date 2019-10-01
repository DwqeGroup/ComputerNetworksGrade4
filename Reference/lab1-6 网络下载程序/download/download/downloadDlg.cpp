
// downloadDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "download.h"
#include "downloadDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CdownloadDlg 对话框
extern int download(char* url, char* path, char* name);



CdownloadDlg::CdownloadDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CdownloadDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CdownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, url_edit);
	DDX_Control(pDX, IDC_EDIT2, path_edit);
	DDX_Control(pDX, IDC_EDIT3, name_edit);
}

BEGIN_MESSAGE_MAP(CdownloadDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CdownloadDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CdownloadDlg 消息处理程序

BOOL CdownloadDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CdownloadDlg::OnPaint()
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
HCURSOR CdownloadDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CdownloadDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	char url[130] = {0};
	char path[260] = {0};
	char name[260] = {0};
	CString text;

	url_edit.GetWindowTextA(text);
	strcpy(url,(LPSTR)(LPCTSTR)text);

	path_edit.GetWindowTextA(text);
	strcpy(path,(LPSTR)(LPCTSTR)text);

	name_edit.GetWindowTextA(text);
	strcpy(name,(LPSTR)(LPCTSTR)text);

	download(url, path, name);
}
