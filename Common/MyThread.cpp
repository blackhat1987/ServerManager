#include "stdafx.h"
#include "MyThread.h"


CMyThread::CMyThread()
{
    m_hThreadHandle = INVALID_HANDLE_VALUE;
}


CMyThread::~CMyThread()
{
    EndThread(INFINITE);
}

//开始线程
bool CMyThread::StartThread()
{
    m_hThreadHandle = (HANDLE)_beginthreadex(NULL, NULL,
        myThreadFunc,
        this, 0, (unsigned*)&m_ThreadID);
    if (m_hThreadHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    return true;
}
//结束线程
bool CMyThread::EndThread(DWORD dwWaitTime)
{
    //释放资源
    if (m_hThreadHandle != INVALID_HANDLE_VALUE)
    {
        WaitForSingleObject(m_hThreadHandle, dwWaitTime);
        CloseHandle(m_hThreadHandle);
    }
    m_hThreadHandle = INVALID_HANDLE_VALUE;
    return true;
}

//线程回调函数
 unsigned CMyThread::myThreadFunc(LPVOID lpParam)
{
    CMyThread* pThis = (CMyThread*)lpParam;

    if (pThis == NULL) return 0;
    bool bRet = pThis->OnThreadEventStart();
    if (!bRet)
    {
        return 0;
    }

    try
    {
        pThis->OnThreadEventRun();
    }
    catch (...)
    {
        ASSERT(FALSE);
    }

    try
    {
        pThis->OnThreadEventEnd();
    }
    catch (...)
    {
        ASSERT(FALSE);
    }

    return 0;
}

 BOOL CMyThread::PostThreadMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
 {
     return ::PostThreadMessage(m_ThreadID, uMsg, wParam, lParam);
 }

 bool CMyThread::OnThreadEventRun()
 {
     MSG    msg;
     BOOL   bRet;
     while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
     {
         if (bRet == -1)
         {
             // handle the error and possibly exit
             return false;
         }
         else
         {
             OnHandleMsg(msg);
         }
     }
     return true;
 }