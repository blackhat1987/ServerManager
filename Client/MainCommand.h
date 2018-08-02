#pragma once
#include "IMyCommand.h"
#include "ClientThreadPool.h"
class CClientSocket;
class CMainCommand :
    public IMyCommand
{
public:
    CMainCommand(char* szIP, int nPort, bool* IsRun);
    virtual ~CMainCommand();
    virtual void OnRecv(char*  pBuffer, int nSize);
    virtual void OnSend(char*  pBuffer, int nSize);
    virtual void OnClose();
    virtual void Execute();

    void SendLoginMessage();
    static unsigned __stdcall HeratProc(LPVOID lpParam);
private:
    CClientSocket* m_pClient;
    CClientSocket   m_ClientSocket;
    CClientThreadPool m_ClientThreadPool;

    char*       m_szIp;
    int         m_nPort;
    HANDLE      m_hEvent;


    HANDLE      m_HeratThread;
    DWORD       m_dwLastTime;
    DWORD       m_dwSpeed;
    bool*       m_IsRun;
};

