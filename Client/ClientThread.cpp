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
            //1. ��������񣬴������б���ȡ������
            IMyCommand* pCmd;
            OutputDebugStringA("Log:�̳߳شӶ��л�ȡ������");
            bRet = m_pManager->Get(pCmd);
            //2. ִ������
            if (bRet && pCmd != NULL)
            {
                OutputDebugStringA("Log:�̳߳�ִ������");
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
