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
    //��ʼ��������
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
        OutputDebugStringA("Log:������Ļʧ��");
        return;
    }
    if (m_HeratThread == INVALID_HANDLE_VALUE)
    {
        return;
    }
    char bCommand = COMMAND_Screen_OPEN;
    OutputDebugStringA("Log:������Ϣ");
    OnSend(&bCommand, sizeof(char));
    WaitForSingleObject(m_hEvent, INFINITE);
}

void CScreenCommand::GetScreen()
{

    CDC dc, bmpdc;                               //��Ļ��������ʱ����
    int width, height;                               //��Ļ�Ŀ�Ⱥ͸߶�
    dc.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);  //������Ļ�����Ĵ�������

    CBitmap bm;                                                    //����洢��Ļͼ���λͼ

    width = GetSystemMetrics(SM_CXSCREEN);                         //��Ļ���

    height = GetSystemMetrics(SM_CYSCREEN);                         //��Ļ�߶�

    bm.CreateCompatibleBitmap(&dc, width, height);                   //��������Ļ���ݵ�λͼ

    bmpdc.CreateCompatibleDC(&dc);    //��������Ļ�������ݵ���ʱ����

    bmpdc.SelectObject(&bm);                                      //ѡ��ͼƬ

    bmpdc.BitBlt(0, 0, width, height, &dc, 0, 0, SRCCOPY);//����Ļͼ���Ƶ�λͼ��

    bm.GetBitmap(&m_bitmap);                                         //��ȡλͼ�ṹ

    m_dwSize = m_bitmap.bmWidthBytes * m_bitmap.bmHeight;               //����λͼ���ݴ�С

    m_bmpdata = new char[m_dwSize];                                       //�����洢λͼ���ݵĻ�����

    if (m_bmpdata == NULL)
    {
        return;
    }
    BITMAPINFOHEADER bih;                                     //λͼ��Ϣͷ

    bih.biBitCount = m_bitmap.bmBitsPixel;                           //��ɫλ��

    bih.biClrImportant = 0;

    bih.biClrUsed = 0;

    bih.biCompression = 0;

    bih.biHeight = m_bitmap.bmHeight;                                 //λͼ�߶�

    bih.biPlanes = 1;

    bih.biSize = sizeof(BITMAPINFOHEADER);

    bih.biSizeImage = m_dwSize;                                               //λͼ��С

    bih.biWidth = m_bitmap.bmWidth;                                  //λͼ���

    bih.biXPelsPerMeter = 0;

    bih.biYPelsPerMeter = 0;

    //��ȡλͼ���ݵ�bmpdata

    GetDIBits(dc, bm, 0, bih.biHeight, m_bmpdata, (BITMAPINFO*)&bih, DIB_RGB_COLORS);

}

void CScreenCommand::SendBitmap()
{
    CBuffer buf;
    GetScreen();
    char Type = COMMAND_Screen_BEGIN;
    buf.Write((PBYTE)&Type, sizeof(char));
    buf.Write((PBYTE)&m_bitmap, sizeof(BITMAP));
    //����ͼƬ�ṹ��
    OnSend((char*)buf.GetBuffer(), buf.GetBufferLen());
}

void CScreenCommand::SendBitData()
{
    CBuffer buf;
    char Type = COMMAND_Screen_DATA;
    buf.Write((PBYTE)&Type, sizeof(char));
    buf.Write((PBYTE)m_bmpdata, m_dwSize);
    //����ͼƬ����
    OnSend((char*)buf.GetBuffer(0), buf.GetBufferLen());
    delete m_bmpdata;
    m_bmpdata = NULL;
}


void CScreenCommand::SendWithSplit(char* pBuf, DWORD dwSize)
{
    //����ּ��η���
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
