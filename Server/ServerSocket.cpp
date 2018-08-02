// ServerSocket.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "serverdlg.h"
#pragma comment(lib,"ws2_32.lib")


// CServerSocket

CServerSocket::CServerSocket():
m_pDlg(NULL)
{
}

CServerSocket::~CServerSocket()
{
}


// CServerSocket 成员函数


void CServerSocket::OnClose(int nErrorCode)
{
    // TODO:  在此添加专用代码和/或调用基类
    m_pDlg->RemoveSocket(this);
    CSocket::OnClose(nErrorCode);
}


void CServerSocket::OnAccept(int nErrorCode)
{
    // TODO:  在此添加专用代码和/或调用基类
    m_pDlg->AddSocket();
    CSocket::OnAccept(nErrorCode);
}


void CServerSocket::OnReceive(int nErrorCode)
{
    // TODO:  在此添加专用代码和/或调用基类
    m_pDlg->ReceiveData(this);
    CSocket::OnReceive(nErrorCode);
}

void CServerSocket::SetDlg(CServerDlg* pDlg)
{
    m_pDlg = pDlg;
}