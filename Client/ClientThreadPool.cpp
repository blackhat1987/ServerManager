#include "stdafx.h"
#include "ClientThreadPool.h"


CClientThreadPool::CClientThreadPool()
{
    CreateCClientThread();
    OutputDebugStringA("Log:初始化线程池完成");
}


CClientThreadPool::~CClientThreadPool()
{
    OutputDebugStringA("Log:等待线程池退出");
    DestroyMyThread();  
}

void CClientThreadPool::CreateCClientThread()
{
    //创建信号量 
    m_hSemaphore = CreateSemaphore(NULL, 0, 0x7fffffff, NULL);
    if (m_hSemaphore == NULL)
    {
        return;
    }
    //初始化线程
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        m_ClientThreadAry[i].InitClientThraead(&m_hSemaphore, &m_Task);
    }
    //初始化线程
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
        OutputDebugStringA("Log:添加一个命令到队列");
        //释放一个信号量
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
    //等待线程
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        m_ClientThreadAry[i].EndThread(10);
    }
    return true;
}