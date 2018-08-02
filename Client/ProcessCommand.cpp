#include "stdafx.h"
#include "ProcessCommand.h"
#include "../Common/common.h"
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>

//����Ȩ��ΪDEBUG,����GetLastError����5 ��Ȩ�޲�������
BOOL EnableDebugPrivilege()
{
    HANDLE hToken;
    BOOL fOk = FALSE;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount = 1;
        LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);

        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);

        fOk = (GetLastError() == ERROR_SUCCESS);
        CloseHandle(hToken);
    }
    return fOk;
}

CProcessCommand::CProcessCommand(char* szIP, int nPort)
{
    m_szIp = szIP;
    m_nPort = nPort;
    m_ClientSocket.SetComand(this);
    m_hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    m_dwLastTime = 0;
    m_HeratThread = INVALID_HANDLE_VALUE;
}


CProcessCommand::~CProcessCommand()
{
    if (m_HeratThread != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_HeratThread);
        m_HeratThread = INVALID_HANDLE_VALUE;
    }
    CloseHandle(m_hEvent);
}

void CProcessCommand::OnRecv(char* pBuffer, int nSize)
{
    switch (pBuffer[0])
    {
    case COMMAND_PROCESS_BEGIN:
        SendProcessData();
        break;
    case COMMAND_PROCESS_KILL:
        KillProcess(pBuffer + 1);
        break;
    case COMMAND_HEARTBEAT:
        m_dwLastTime = GetTickCount() / 1000;
        Dbgprintf("�յ�����");
        break;
    default:
        break;
    }
}

void CProcessCommand::OnSend(char* pBuffer, int nSize)
{
    m_ClientSocket.OnSend(pBuffer, nSize);
}

unsigned __stdcall CProcessCommand::HeratProc(LPVOID lpParam)
{
    CProcessCommand* pThis = (CProcessCommand*)lpParam;
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


void CProcessCommand::Execute()
{
    OutputDebugStringA("Log:ProcessCmd��ʼִ��");
    bool bRet = m_ClientSocket.ConnectServer(m_szIp, m_nPort);
    if (!bRet)
    {
        return;
    }
    m_dwLastTime = GetTickCount() / 1000;
    char Type = COMMAND_PROCESS_OPEN;
    OutputDebugStringA("Log:���ʹ�Process������Ϣ");
    OnSend(&Type, sizeof(char));
    //�������߳�
    //m_HeratThread = (HANDLE)_beginthreadex(NULL, 0, HeratProc, this, 0, NULL);
    //if (m_HeratThread == INVALID_HANDLE_VALUE)
    //{
    //    return;
    //}
    //�ȴ��¼�
    WaitForSingleObject(m_hEvent, INFINITE);
    OutputDebugStringA("Log:CProcessCommand::Execute()ִ�����");
}

void CProcessCommand::OnClose()
{
    SetEvent(m_hEvent);
}

void CProcessCommand::SendProcessData()
{
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    int nProcessCount = 0;
    CBuffer pData;
    CBuffer pSendData;
    BOOL bRet;
    //��Ȩ
    EnableDebugPrivilege();
    //�Ȼ�ȡ���̿���
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hProcessSnap)
    {
        return;
    }
    //ʹ��֮ǰҪ����д�ṹ���С
    pe32.dwSize = sizeof(PROCESSENTRY32);
    bRet = Process32First(hProcessSnap, &pe32);
    if (!bRet)
    {
        CloseHandle(hProcessSnap);
        return;
    }
    //��������
    do
    {
        char ch;
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
        if (hProcess != NULL)
        {
            ch = true;
        }
        else
        {
            ch = false;
        }
        CString csTemp = pe32.szExeFile;
        //����Ҳ�����ʻ���
        string str = UnicodeToAscii(csTemp.GetString());
        pData.Write((PBYTE)str.data(), str.length()+1);
        pData.Write((PBYTE)&pe32.th32ProcessID, sizeof(pe32.th32ProcessID));
        pData.Write((PBYTE)&ch, sizeof(char));
        CloseHandle(hProcess);
        nProcessCount++;
    } while (Process32Next(hProcessSnap, &pe32));
    char Type = COMMAND_PROCESS_DATA;
    pSendData.Write((PBYTE)&Type, sizeof(char));
    //д���������
    pSendData.Write((PBYTE)&nProcessCount, sizeof(int));
    pSendData.Write(pData.GetBuffer(0), pData.GetBufferLen()+1);
    OnSend((char*)pSendData.GetBuffer(0), pSendData.GetBufferLen());
}

void CProcessCommand::KillProcess(char* pBuffer)
{
    DWORD dwPid = *(DWORD*)pBuffer;
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
    BOOL bRet = TerminateProcess(hProcess, NULL);
    char Type;
    if (bRet)
    {
        Type = COMMAND_PROCESS_KILL_Success;
    }
    else
    {
        Type = COMMAND_PROCESS_KILL_Fail;
    }
    OnSend(&Type, sizeof(char));
}
