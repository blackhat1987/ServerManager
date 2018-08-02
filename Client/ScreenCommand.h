#pragma once
#include "IMyCommand.h"
#include "ClientSocket.h"
class CScreenCommand :
    public IMyCommand
{
public:
    CScreenCommand(char* szIP, int nPort);
    virtual ~CScreenCommand();
    virtual void OnRecv(char*  pBuffer, int nSize);
    virtual void OnSend(char*  pBuffer, int nSize);
    virtual void OnClose();
    virtual void Execute();

    void GetScreen();               //获取屏幕截图
    void SendBitmap();              //先发送位图结构体
    void SendBitData();             //发送图片
    void SendWithSplit(char* pBuf, DWORD dwSize);//切块发送


    static unsigned __stdcall HeratProc(LPVOID lpParam);

private:
    CClientSocket m_ClientSocket;
    char*       m_szIp;
    int         m_nPort;

    char*       m_bmpdata;
    BITMAP      m_bitmap;
    DWORD       m_dwSize;

    HANDLE      m_hEvent;
    HANDLE      m_HeratThread;
    DWORD       m_dwLastTime;
};

