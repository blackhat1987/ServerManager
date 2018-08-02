#pragma once
#include "IOCPServer.h"
#include "../Common/common.h"
#include "../Common/DataStruct.h"
#include "afxwin.h"

// CScreenDlg 对话框

class CScreenDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CScreenDlg)

public:
    CScreenDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, tagClientContext* pClientContext = NULL);   // 标准构造函数
	virtual ~CScreenDlg();

// 对话框数据
	enum { IDD = ScreenDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    virtual BOOL OnInitDialog();
    afx_msg void OnClose();

    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedButton2();


    void OnReceive();
    void OnSend();      

    void DrawScreen();  //绘制屏幕

    void GetBitMap();  //获取图像结构
    void GetData();    //获取图像数据

    CIOCPServer*        m_pIOCPServer;
    tagClientContext*   m_pClientContext;


    DWORD       m_dwSize;
    char*       m_bmpdata;
    BITMAP      m_bitmap;

    CString m_csIp;
    CStatic m_drawscreen;
    bool    m_IsSotp;  //停止发送屏幕

};
