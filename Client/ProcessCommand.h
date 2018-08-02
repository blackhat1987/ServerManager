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

    void    SendProcessData(); //ö�ٽ��̲�����
    void    KillProcess(char* pBuffer); //��������
    static unsigned __stdcall HeratProc(LPVOID lpParam); //��������߳�
private:
    CClientSocket   m_ClientSocket;
    char*           m_szIp;
    int             m_nPort;
    HANDLE          m_hEvent; //�̳߳�ͬ����



    HANDLE      m_HeratThread;
    DWORD       m_dwLastTime;
};

