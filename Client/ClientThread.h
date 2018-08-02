#pragma once
#include "../Common/MyThread.h"
#include "TaskManager.h"
#include "ClientSocket.h"
class CClientThread :
    public CMyThread
{
public:
    CClientThread();
    virtual ~CClientThread();
    virtual bool OnThreadEventRun();
    void InitClientThraead(HANDLE* pSem, CTaskManager* pManager);
private:
    HANDLE* m_pSem;
    CTaskManager* m_pManager;
};

