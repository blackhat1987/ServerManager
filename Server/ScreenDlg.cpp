// ScreenDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "ScreenDlg.h"
#include "afxdialogex.h"


// CScreenDlg �Ի���

IMPLEMENT_DYNAMIC(CScreenDlg, CDialogEx)

CScreenDlg::CScreenDlg(CWnd* pParent, CIOCPServer* pIOCPServer, tagClientContext* pClientContext)
	: CDialogEx(CScreenDlg::IDD, pParent)
{
    m_pIOCPServer = pIOCPServer;
    m_pClientContext = pClientContext;
    m_csIp = inet_ntoa(pClientContext->addr.sin_addr);
}

CScreenDlg::~CScreenDlg()
{
}

void CScreenDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_P, m_drawscreen);
}


BEGIN_MESSAGE_MAP(CScreenDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &CScreenDlg::OnBnClickedOk)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BUTTON1, &CScreenDlg::OnBnClickedButton1)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON2, &CScreenDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CScreenDlg ��Ϣ�������


void CScreenDlg::OnBnClickedOk()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    //CDialogEx::OnOK();
}


BOOL CScreenDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  �ڴ���Ӷ���ĳ�ʼ��
    m_IsSotp = true;

    CString str;
    str.Format(_T("%s---��Ļ���"), m_csIp);
    SetWindowText(str);

    return TRUE;  // return TRUE unless you set the focus to a control
    // �쳣:  OCX ����ҳӦ���� FALSE
}


void CScreenDlg::OnClose()
{
    // TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
    m_pClientContext->m_Dialog[0] = 0;
    m_pClientContext->m_Dialog[1] = 0;
    shutdown(m_pClientContext->s, 0);
    closesocket(m_pClientContext->s);
    CDialogEx::OnClose();
}

void CScreenDlg::OnReceive()
{
    char Type;
    m_pClientContext->m_CompressBuffer.Read((PBYTE)&Type, sizeof(char));
    m_pClientContext->dwLastTime = GetTickCount() / 1000;
    switch (Type)
    {
    case COMMAND_Screen_BEGIN://ͼƬ�ṹ��
        GetBitMap();
        break;
    case COMMAND_Screen_DATA: //ͼƬ����
        GetData();
        break;
    default:
        break;
    }
}

void CScreenDlg::OnSend()
{
    //���߿ͻ�����׼������
    char ch = COMMAND_Screen_BEGIN;
    m_pIOCPServer->Send(m_pClientContext, &ch, sizeof(char));
}


void CScreenDlg::OnBnClickedButton1()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    CImage  imag;
    CDC *pdc = m_drawscreen.GetDC();
    CRect rect;
    m_drawscreen.GetWindowRect(&rect);
    imag.Create(rect.Width(), rect.Height(), 32);
    ::BitBlt(imag.GetDC(), 0, 0, rect.Width(), rect.Height(), pdc->m_hDC, 0, 0, SRCCOPY);

    CTime currTime = CTime::GetCurrentTime();
    CString strTime;
    strTime.Format(_T("%.4d%.2d%.2d%.2d%.2d%.2d"), currTime.GetYear(), currTime.GetMonth(), currTime.GetDay(), currTime.GetHour(), currTime.GetMinute(), currTime.GetSecond());
    HRESULT hResult = imag.Save(_T("./")+strTime + _T(".bmp")); //����ͼƬ
    ReleaseDC(pdc);
    imag.ReleaseDC();
}


void CScreenDlg::DrawScreen()

{

    CDC *dc = m_drawscreen.GetDC();                                 //Picture�ؼ�����

    BITMAPINFOHEADER bih;                                      //λͼ��Ϣͷ

    bih.biBitCount = m_bitmap.bmBitsPixel;                                   //��ɫλ��

    bih.biClrImportant = 0;

    bih.biClrUsed = 0;

    bih.biCompression = 0;

    bih.biHeight = m_bitmap.bmHeight;                                  //λͼ�߶�

    bih.biPlanes = 1;

    bih.biSize = sizeof(BITMAPINFOHEADER);

    bih.biSizeImage = m_dwSize;                                                //λͼ��С

    bih.biWidth = m_bitmap.bmWidth;                                   //λͼ���

    bih.biXPelsPerMeter = 0;

    bih.biYPelsPerMeter = 0;



    CBitmap bm;

    bm.CreateBitmapIndirect(&m_bitmap);                    //����λͼ�ṹ����λͼ

    CDC bmpdc;

    bmpdc.CreateCompatibleDC(dc);

    SetDIBits(bmpdc.m_hDC, bm, 0, m_bitmap.bmHeight, m_bmpdata,(BITMAPINFO*)&bih, DIB_RGB_COLORS); //��λͼ���������

    bmpdc.SelectObject(&bm);                                       //ѡ��λͼ

    CRect rect;

    m_drawscreen.GetClientRect(&rect);

    //��ͼ����Ƶ�Picture�ؼ���

    dc->StretchBlt(0, 0, rect.Width(), rect.Height(), &bmpdc, 0, 0, m_bitmap.bmWidth, m_bitmap.bmHeight, SRCCOPY);
    ReleaseDC(dc);
}

void CScreenDlg::GetBitMap()
{
    if (m_pClientContext->m_CompressBuffer.GetBufferLen()<=0)
    {
        return;
    }
    memcpy(&m_bitmap, m_pClientContext->m_CompressBuffer.GetBuffer(), sizeof(BITMAP));
    m_dwSize = m_bitmap.bmWidthBytes * m_bitmap.bmHeight;//����ͼ���С
    m_bmpdata = new char[m_dwSize];
    if (m_bmpdata == NULL)
    {
        return;
    }
    memset(m_bmpdata, 0, m_dwSize);
    char Type = COMMAND_Screen_DATA;
    m_pIOCPServer->Send(m_pClientContext, &Type, sizeof(char));
}

void CScreenDlg::GetData()
{
    if (m_pClientContext->m_CompressBuffer.GetBufferLen() <= 0)
    {
        return;
    }
    memcpy(m_bmpdata, m_pClientContext->m_CompressBuffer.GetBuffer(), m_dwSize);
    DrawScreen();
    delete[] m_bmpdata;
    m_bmpdata = NULL;
    //���ڼ�������һ֡
    if (m_IsSotp)
    {
        OnSend();
    }
}



void CScreenDlg::OnBnClickedButton2()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    m_IsSotp = false;
}

