

#include "../Common/common.h"
#include "ClientSocket.h"
#include "MainCommand.h"
// ClientDlg.h : ͷ�ļ�
//

#pragma once


// CClientDlg �Ի���
class CClientDlg : public CDialogEx
{
// ����
public:
	CClientDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_CLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
    
    //��ʱ��
    CString     m_csIP;
    CString     m_csPort;

    HANDLE      m_MainThread;

    bool        m_IsRun;//ж����
};
