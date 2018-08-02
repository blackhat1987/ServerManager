// ServerSocket.cpp : ʵ���ļ�
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


// CServerSocket ��Ա����


void CServerSocket::OnClose(int nErrorCode)
{
    // TODO:  �ڴ����ר�ô����/����û���
    m_pDlg->RemoveSocket(this);
    CSocket::OnClose(nErrorCode);
}


void CServerSocket::OnAccept(int nErrorCode)
{
    // TODO:  �ڴ����ר�ô����/����û���
    m_pDlg->AddSocket();
    CSocket::OnAccept(nErrorCode);
}


void CServerSocket::OnReceive(int nErrorCode)
{
    // TODO:  �ڴ����ר�ô����/����û���
    m_pDlg->ReceiveData(this);
    CSocket::OnReceive(nErrorCode);
}

void CServerSocket::SetDlg(CServerDlg* pDlg)
{
    m_pDlg = pDlg;
}