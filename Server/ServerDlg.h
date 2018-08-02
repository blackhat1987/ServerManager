
#include "SEU_QQwry.h"
#include "IOCPServer.h"
#include "afxcmn.h"
// ServerDlg.h : 头文件
//
#include "FileDlg.h"
#include "ShellDlg.h"
#include "ScreenDlg.h"
#include "ProcessDlg.h"
#include "SoftDlg.h"

enum
{
    Shell_Dlg = 1,
    File_Dlg,
    Screen_Dlg,
    Process_Dlg
};
#pragma once

// CServerDlg 对话框
class CServerDlg : public CDialogEx
{
// 构造
public:
	CServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();


    afx_msg void OnFileManage();
    afx_msg void OnScreen();
    afx_msg void OnProcess();
    afx_msg void OnCamera();
    afx_msg void OnShellMange();
    afx_msg void OnSoftSet();
    afx_msg void OnAboutDlg();
    afx_msg void OnExit();
	DECLARE_MESSAGE_MAP()
    //托盘
    afx_msg void AddSystrayIcon();
    afx_msg void DelSystrayIcon();
    //定义自己的消息处理
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    //列表框右键
    afx_msg void OnRclickList1(NMHDR *pNMHDR, LRESULT *pResult);
    //时钟
    afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
    //启动
    void StartServer();
    //停止
    void Stop();
    //IOCP消息回调处理函数
    static void CALLBACK NotifyProc(LPVOID lpParam, tagClientContext* pContext, UINT nCode);

    //处理正常消息
    void ProcessMessage(tagClientContext* pContext);
    //给选中的发送命令
    void SelectComamndSend(char*  pBuffer, int nSize);

    CListCtrl          m_ListCtrl;
    CToolBar           m_ToolBar;
    CImageList         m_ImageList;
    CStatusBar         m_StatusBar;
    BOOL               m_bIsQQwryExist;
    SEU_QQwry*         m_QQwry;
    CTime              time;
    NOTIFYICONDATA     g_nd;//托盘

    CString            m_csPort;
    
    CIOCPServer*       m_IOCPServer;
    
protected:
    afx_msg LRESULT OnOpenshelldlg(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnOpenfiledlg(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnOpensrceendlg(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnOpenprocessdlg(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnReplyhreat(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAddlist(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnRemovelist(WPARAM wParam, LPARAM lParam);
public:
    virtual BOOL DestroyWindow();

    afx_msg void OnUninstall();
};
