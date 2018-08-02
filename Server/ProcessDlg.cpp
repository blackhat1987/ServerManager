// ProcessDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "ProcessDlg.h"
#include "afxdialogex.h"


// CProcessDlg �Ի���

IMPLEMENT_DYNAMIC(CProcessDlg, CDialogEx)

CProcessDlg::CProcessDlg(CWnd* pParent, CIOCPServer* pIOCPServer, tagClientContext* pClientContext)
	: CDialogEx(CProcessDlg::IDD, pParent)
{
    m_pIOCPServer = pIOCPServer;
    m_pClientContext = pClientContext;
    m_csIp = inet_ntoa(pClientContext->addr.sin_addr);
}

CProcessDlg::~CProcessDlg()
{
}

void CProcessDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_ProcessList);
}


BEGIN_MESSAGE_MAP(CProcessDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &CProcessDlg::OnBnClickedOk)
    ON_WM_CLOSE()
    ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CProcessDlg::OnNMRClickList1)
    ON_COMMAND(BN_KILLPROCESS, &CProcessDlg::OnKillprocess)
    ON_COMMAND(MN_Update, &CProcessDlg::OnUpdate)
END_MESSAGE_MAP()


// CProcessDlg ��Ϣ�������


void CProcessDlg::OnBnClickedOk()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    //CDialogEx::OnOK();
}


BOOL CProcessDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  �ڴ���Ӷ���ĳ�ʼ��


    CString str;
    str.Format(_T("%s---���̹���"), m_csIp);
    SetWindowText(str);


    m_ProcessList.InsertColumn(0, _T("������"), LVCFMT_LEFT, 220);
    m_ProcessList.InsertColumn(1, _T("����ID"), LVCFMT_LEFT, 80);
    m_ProcessList.InsertColumn(2, _T("����Ȩ��"), LVCFMT_LEFT, 80);
    //������չ���
    DWORD dwStyle = m_ProcessList.GetExtendedStyle();
    m_ProcessList.SetExtendedStyle(dwStyle | LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    return TRUE;  // return TRUE unless you set the focus to a control
    // �쳣:  OCX ����ҳӦ���� FALSE
}


void CProcessDlg::OnClose()
{
    // TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
    m_pClientContext->m_Dialog[0] = 0;
    m_pClientContext->m_Dialog[1] = 0;
    shutdown(m_pClientContext->s, 0);
    closesocket(m_pClientContext->s);
    CDialogEx::OnClose();
}

void CProcessDlg::OnReceive()
{
    char Type;
    m_pClientContext->m_CompressBuffer.Read((PBYTE)&Type, sizeof(char));
    switch (Type)
    {
    case COMMAND_PROCESS_DATA:
        //��������
        GetProcessData();
        break;
    case COMMAND_PROCESS_KILL_Success:
        AfxMessageBox(_T("�������̳ɹ�")); //�ֶ�ˢ�� �Զ�ˢ�±���BUG
        Sleep(500);
        OnSend();
        break;
    case COMMAND_PROCESS_KILL_Fail:
        AfxMessageBox(_T("��������ʧ��")); //�ֶ�ˢ�� �Զ�ˢ�±���BUG
        break;
    default:
        break;
    }
}

void CProcessDlg::OnSend()
{
    //���߿ͻ�����׼������
    char Type = COMMAND_PROCESS_BEGIN;
    m_pIOCPServer->Send(m_pClientContext, &Type, sizeof(char));
}


void CProcessDlg::GetProcessData()
{
    //��������������
    int nProcessCount = *(int*)m_pClientContext->m_CompressBuffer.GetBuffer(0);
    //����������ֵ
    char* pBuf = (char*)m_pClientContext->m_CompressBuffer.GetBuffer(sizeof(int));
    m_ProcessList.DeleteAllItems();
    //������  ID Ȩ�� = �䳤+4+1
    for (int i = 0; i < nProcessCount;i++)
    {
        CString csTemp;
        CString csProcessId;
        CString csProcessName(pBuf);
        int nLength = strlen(pBuf)+1;
        pBuf += nLength;
        DWORD nPid = *(DWORD*)pBuf;
        csProcessId.Format(_T("%d"), nPid);
        pBuf += sizeof(DWORD);
        if (*pBuf)
        {
            csTemp = _T("�������");
        }
        else
        {
            csTemp = _T("�ܾ�����");
        }
        int nIndex = m_ProcessList.GetItemCount();
        m_ProcessList.InsertItem(nIndex, _T(""));
        m_ProcessList.SetItemText(nIndex, 0, csProcessName);
        m_ProcessList.SetItemText(nIndex, 1, csProcessId);
        m_ProcessList.SetItemText(nIndex, 2, csTemp);
        m_ProcessList.SetItemData(nIndex, nPid);
        pBuf++;
    }
}

void CProcessDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    int nIndex = pNMItemActivate->iItem;
    if (nIndex != -1)
    {
        CMenu menu;
        menu.LoadMenu(MN_PROCESS_RIGHTKEY);
        CMenu* pMenu = menu.GetSubMenu(0);
        CPoint pos;
        GetCursorPos(&pos);
        ::SetForegroundWindow(m_hWnd);
        pMenu->TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this);
    }
    *pResult = 0;
}


void CProcessDlg::OnKillprocess()
{
    // TODO:  �ڴ���������������
    int nIndex = m_ProcessList.GetSelectionMark();
    if (nIndex != -1)
    {
        CString csTemp;
        csTemp = m_ProcessList.GetItemText(nIndex, 2);
        if (csTemp != _T("�������"))
        {
            AfxMessageBox(_T("�ܾ�����"));
            return;
        }
        DWORD dwProcessId = m_ProcessList.GetItemData(nIndex);
        int nPacket = sizeof(char) + sizeof(DWORD);//1+4
        char* pBuf = new char[nPacket];
        if (pBuf == NULL)
        {
            AfxMessageBox(_T("xxxxxx"));
            return;
        }
        memset(pBuf, 0, nPacket);
        pBuf[0] = COMMAND_PROCESS_KILL;
        memcpy(pBuf + 1, &dwProcessId, sizeof(DWORD));
        m_pIOCPServer->Send(m_pClientContext, pBuf, sizeof(nPacket));
    }
}


void CProcessDlg::OnUpdate()
{
    // TODO:  �ڴ���������������
    OnSend();
}
