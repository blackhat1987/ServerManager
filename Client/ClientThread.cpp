#include "stdafx.h"
#include "ClientThread.h"


CClientThread::CClientThread()
    :m_pSem(NULL),
    m_pManager(NULL)
{

}


CClientThread::~CClientThread()
{
}

bool CClientThread::OnThreadEventRun()
{
    BOOL bRet = FALSE;
    while (true)
    {
        DWORD dwRet = WaitForSingleObject(*m_pSem, INFINITE);
        if (dwRet == WAIT_FAILED)
        {
            return false;
        }
        else if (dwRet == WAIT_OBJECT_0)
        {
            //1. 如果有任务，从任务列表中取出任务
            IMyCommand* pCmd;
            OutputDebugStringA("Log:线程池从丢列获取到命令");
            bRet = m_pManager->Get(pCmd);
            //2. 执行任务
            if (bRet && pCmd != NULL)
            {
                OutputDebugStringA("Log:线程池执行命令");
                pCmd->Execute();
                delete pCmd;
            }
        }
    }
    return 1;
}

void CClientThread::InitClientThraead(HANDLE* pSem, CTaskManager* pManager)
{
    m_pSem = pSem;
    m_pManager = pManager;
}
