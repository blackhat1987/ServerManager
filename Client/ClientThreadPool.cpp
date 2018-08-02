#include "stdafx.h"
#include "ClientThreadPool.h"


CClientThreadPool::CClientThreadPool()
{
    CreateCClientThread();
    OutputDebugStringA("Log:��ʼ���̳߳����");
}


CClientThreadPool::~CClientThreadPool()
{
    OutputDebugStringA("Log:�ȴ��̳߳��˳�");
    DestroyMyThread();  
}

void CClientThreadPool::CreateCClientThread()
{
    //�����ź��� 
    m_hSemaphore = CreateSemaphore(NULL, 0, 0x7fffffff, NULL);
    if (m_hSemaphore == NULL)
    {
        return;
    }
    //��ʼ���߳�
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        m_ClientThreadAry[i].InitClientThraead(&m_hSemaphore, &m_Task);
    }
    //��ʼ���߳�
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        m_ClientThreadAry[i].StartThread();
    }
}

bool CClientThreadPool::PostAddCmd(IMyCommand* pCmd)
{
    if (pCmd != NULL)
    {
        m_Task.Add(pCmd);
        OutputDebugStringA("Log:���һ���������");
        //�ͷ�һ���ź���
        ReleaseSemaphore(m_hSemaphore, 1, NULL);
    }
    return true;
}

bool CClientThreadPool::DestroyMyThread()
{
    if (m_hSemaphore != NULL)
    {
        CloseHandle(m_hSemaphore);
        m_hSemaphore = NULL;
    }
    //�ȴ��߳�
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        m_ClientThreadAry[i].EndThread(10);
    }
    return true;
}