// SoftDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "SoftDlg.h"
#include "afxdialogex.h"


// CSoftDlg �Ի���

IMPLEMENT_DYNAMIC(CSoftDlg, CDialogEx)

CSoftDlg::CSoftDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSoftDlg::IDD, pParent)
    , m_csPort(_T("2018"))
{

}

CSoftDlg::~CSoftDlg()
{
}

void CSoftDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, EDIT_PORT, m_csPort);
}


BEGIN_MESSAGE_MAP(CSoftDlg, CDialogEx)
    ON_BN_CLICKED(IDCANCEL, &CSoftDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDOK, &CSoftDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSoftDlg ��Ϣ�������


void CSoftDlg::OnBnClickedCancel()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    CDialogEx::OnCancel();
}


void CSoftDlg::OnBnClickedOk()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    GetDlgItemText(EDIT_PORT, m_csPort);
    ::WritePrivateProfileString(_T("����"), _T("PORT"), m_csPort, _T("./config.ini"));
    CDialogEx::OnOK();
}
