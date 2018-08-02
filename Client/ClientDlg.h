

#include "../Common/common.h"
#include "ClientSocket.h"
#include "MainCommand.h"
// ClientDlg.h : 头文件
//

#pragma once


// CClientDlg 对话框
class CClientDlg : public CDialogEx
{
// 构造
public:
	CClientDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    virtual BOOL DestroyWindow();


    static unsigned __stdcall Connect(LPVOID lpParam);

    bool ReadIP();
    bool InitSocket();
    bool ConnectServer();
    bool OnStart();
    
    //临时的
    CString     m_csIP;
    CString     m_csPort;

    HANDLE      m_MainThread;

    bool        m_IsRun;//卸载用
};
