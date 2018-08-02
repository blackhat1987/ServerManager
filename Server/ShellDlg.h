#pragma once
#include "afxwin.h"


// CShellDlg 对话框
class CIOCPServer;
struct tagClientContext;
class CShellDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CShellDlg)

public:
    CShellDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, tagClientContext* pClientContext = NULL);   // 标准构造函数
	virtual ~CShellDlg();

// 对话框数据
	enum { IDD = ShellDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnBnClickedSendcommand();
    afx_msg void OnClose();

    CIOCPServer*        m_pIOCPServer;
    tagClientContext*   m_pClientContext;
    
    CComboBox           m_ComBox;
    CEdit               m_csText;
    CString             m_csIp;

    void OnReceive();
    void OnSend();

    void GetData();
};
