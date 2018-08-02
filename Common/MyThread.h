#pragma once
#include <windows.h>
class CMyThread
{
public:
    CMyThread();
    virtual ~CMyThread();

    //开始线程
    virtual bool StartThread();

    //结束线程
    virtual bool EndThread(DWORD dwWaitTime = INFINITE);

    //线程执行主体
    virtual bool OnThreadEventStart() { return true; };
    virtual bool OnThreadEventRun();
    virtual bool OnThreadEventEnd() { return true; };
    virtual bool OnHandleMsg(MSG msg){ return true; };

    BOOL PostThreadMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    //线程回调函数
    static unsigned __stdcall myThreadFunc(LPVOID lpParam);
private:
    HANDLE m_hThreadHandle;
    DWORD m_ThreadID;
};

