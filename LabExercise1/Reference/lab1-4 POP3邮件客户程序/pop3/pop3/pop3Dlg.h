
// pop3Dlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "pop_socket.h"


// Cpop3Dlg 对话框
class Cpop3Dlg : public CDialogEx
{
// 构造
public:
	Cpop3Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_POP3_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	POP_SOCKET POP3;
	CEdit cedit_popserver;
	CEdit cedit_user;
	CEdit cedit_psw;
	CListBox clistbox_mail;
	afx_msg void OnBnClickedButton2();
	CEdit cedit_cont;
	afx_msg void OnBnClickedButton3();
	afx_msg void OnLbnSelchangeList1();
	CButton cbutton_open;
	CButton cbutton_dele;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	CButton cbutton_fresh;
};
