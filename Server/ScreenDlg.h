#pragma once
#include "IOCPServer.h"
#include "../Common/common.h"
#include "../Common/DataStruct.h"
#include "afxwin.h"

// CScreenDlg �Ի���

class CScreenDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CScreenDlg)

public:
    CScreenDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, tagClientContext* pClientContext = NULL);   // ��׼���캯��
	virtual ~CScreenDlg();

// �Ի�������
	enum { IDD = ScreenDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    virtual BOOL OnInitDialog();
    afx_msg void OnClose();

    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedButton2();


    void OnReceive();
    void OnSend();      

    void DrawScreen();  //������Ļ

    void GetBitMap();  //��ȡͼ��ṹ
    void GetData();    //��ȡͼ������

    CIOCPServer*        m_pIOCPServer;
    tagClientContext*   m_pClientContext;


    DWORD       m_dwSize;
    char*       m_bmpdata;
    BITMAP      m_bitmap;

    CString m_csIp;
    CStatic m_drawscreen;
    bool    m_IsSotp;  //ֹͣ������Ļ

};
