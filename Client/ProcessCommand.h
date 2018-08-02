#pragma once
#include "IMyCommand.h"
#include "ClientSocket.h"
class CProcessCommand :
    public IMyCommand
{
public:
    CProcessCommand(char* szIP, int nPort);
    virtual ~CProcessCommand();
    virtual void OnRecv(char*  pBuffer, int nSize);
    virtual void OnSend(char*  pBuffer, int nSize);
    virtual void Execute();
    virtual void OnClose();

    void    SendProcessData(); //枚举进程并发送
    void    KillProcess(char* pBuffer); //结束进程
    static unsigned __stdcall HeratProc(LPVOID lpParam); //心跳检测线程
private:
    CClientSocket   m_ClientSocket;
    char*           m_szIp;
    int             m_nPort;
    HANDLE          m_hEvent; //线程池同步用



    HANDLE      m_HeratThread;
    DWORD       m_dwLastTime;
};

