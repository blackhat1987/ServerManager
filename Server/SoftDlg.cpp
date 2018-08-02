// SoftDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "SoftDlg.h"
#include "afxdialogex.h"


// CSoftDlg 对话框

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


// CSoftDlg 消息处理程序


void CSoftDlg::OnBnClickedCancel()
{
    // TODO:  在此添加控件通知处理程序代码
    CDialogEx::OnCancel();
}


void CSoftDlg::OnBnClickedOk()
{
    // TODO:  在此添加控件通知处理程序代码
    GetDlgItemText(EDIT_PORT, m_csPort);
    ::WritePrivateProfileString(_T("配置"), _T("PORT"), m_csPort, _T("./config.ini"));
    CDialogEx::OnOK();
}
