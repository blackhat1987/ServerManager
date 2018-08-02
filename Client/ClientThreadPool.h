#pragma once
#include "ClientThread.h"
#include "TaskManager.h"
#define THREAD_COUNT 4
class CClientThreadPool
{
public:
    CClientThreadPool();
    ~CClientThreadPool();
    void CreateCClientThread();
    bool DestroyMyThread();
    bool PostAddCmd(IMyCommand* pCmd);
private:
    CTaskManager m_Task;
    HANDLE m_hSemaphore;
    CClientThread m_ClientThreadAry[THREAD_COUNT];
};

