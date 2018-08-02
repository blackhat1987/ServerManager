// ScreenDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "ScreenDlg.h"
#include "afxdialogex.h"


// CScreenDlg 对话框

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


// CScreenDlg 消息处理程序


void CScreenDlg::OnBnClickedOk()
{
    // TODO:  在此添加控件通知处理程序代码
    //CDialogEx::OnOK();
}


BOOL CScreenDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  在此添加额外的初始化
    m_IsSotp = true;

    CString str;
    str.Format(_T("%s---屏幕监控"), m_csIp);
    SetWindowText(str);

    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常:  OCX 属性页应返回 FALSE
}


void CScreenDlg::OnClose()
{
    // TODO:  在此添加消息处理程序代码和/或调用默认值
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
    case COMMAND_Screen_BEGIN://图片结构体
        GetBitMap();
        break;
    case COMMAND_Screen_DATA: //图片数据
        GetData();
        break;
    default:
        break;
    }
}

void CScreenDlg::OnSend()
{
    //告诉客户端我准备好了
    char ch = COMMAND_Screen_BEGIN;
    m_pIOCPServer->Send(m_pClientContext, &ch, sizeof(char));
}


void CScreenDlg::OnBnClickedButton1()
{
    // TODO:  在此添加控件通知处理程序代码
    CImage  imag;
    CDC *pdc = m_drawscreen.GetDC();
    CRect rect;
    m_drawscreen.GetWindowRect(&rect);
    imag.Create(rect.Width(), rect.Height(), 32);
    ::BitBlt(imag.GetDC(), 0, 0, rect.Width(), rect.Height(), pdc->m_hDC, 0, 0, SRCCOPY);

    CTime currTime = CTime::GetCurrentTime();
    CString strTime;
    strTime.Format(_T("%.4d%.2d%.2d%.2d%.2d%.2d"), currTime.GetYear(), currTime.GetMonth(), currTime.GetDay(), currTime.GetHour(), currTime.GetMinute(), currTime.GetSecond());
    HRESULT hResult = imag.Save(_T("./")+strTime + _T(".bmp")); //保存图片
    ReleaseDC(pdc);
    imag.ReleaseDC();
}


void CScreenDlg::DrawScreen()

{

    CDC *dc = m_drawscreen.GetDC();                                 //Picture控件画布

    BITMAPINFOHEADER bih;                                      //位图信息头

    bih.biBitCount = m_bitmap.bmBitsPixel;                                   //颜色位数

    bih.biClrImportant = 0;

    bih.biClrUsed = 0;

    bih.biCompression = 0;

    bih.biHeight = m_bitmap.bmHeight;                                  //位图高度

    bih.biPlanes = 1;

    bih.biSize = sizeof(BITMAPINFOHEADER);

    bih.biSizeImage = m_dwSize;                                                //位图大小

    bih.biWidth = m_bitmap.bmWidth;                                   //位图宽度

    bih.biXPelsPerMeter = 0;

    bih.biYPelsPerMeter = 0;



    CBitmap bm;

    bm.CreateBitmapIndirect(&m_bitmap);                    //跟据位图结构创建位图

    CDC bmpdc;

    bmpdc.CreateCompatibleDC(dc);

    SetDIBits(bmpdc.m_hDC, bm, 0, m_bitmap.bmHeight, m_bmpdata,(BITMAPINFO*)&bih, DIB_RGB_COLORS); //向位图中添加数据

    bmpdc.SelectObject(&bm);                                       //选择位图

    CRect rect;

    m_drawscreen.GetClientRect(&rect);

    //将图像绘制到Picture控件中

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
    m_dwSize = m_bitmap.bmWidthBytes * m_bitmap.bmHeight;//计算图像大小
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
    //等于假请求下一帧
    if (m_IsSotp)
    {
        OnSend();
    }
}



void CScreenDlg::OnBnClickedButton2()
{
    // TODO:  在此添加控件通知处理程序代码
    m_IsSotp = false;
}

