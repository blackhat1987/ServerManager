#pragma once
#include "afxwin.h"


// CShellDlg �Ի���
class CIOCPServer;
struct tagClientContext;
class CShellDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CShellDlg)

public:
    CShellDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, tagClientContext* pClientContext = NULL);   // ��׼���캯��
	virtual ~CShellDlg();

// �Ի�������
	enum { IDD = ShellDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
