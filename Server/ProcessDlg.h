#pragma once
#include "afxcmn.h"
#include "IOCPServer.h"
#include "../Common/DataStruct.h"

// CProcessDlg �Ի���

class CProcessDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CProcessDlg)

public:
    CProcessDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, tagClientContext* pClientContext = NULL);   // ��׼���캯��
	virtual ~CProcessDlg();

// �Ի�������
	enum { IDD = ProcessDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    virtual BOOL OnInitDialog();
    afx_msg void OnClose();
    afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnKillprocess();
    afx_msg void OnUpdate();

    CListCtrl           m_ProcessList;
    CIOCPServer*        m_pIOCPServer;
    tagClientContext*   m_pClientContext;

    void OnReceive();
    void OnSend();

    void  GetProcessData();

    CString m_csIp;
};
