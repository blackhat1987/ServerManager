#pragma once
#include <windows.h>
class CMyThread
{
public:
    CMyThread();
    virtual ~CMyThread();

    //��ʼ�߳�
    virtual bool StartThread();

    //�����߳�
    virtual bool EndThread(DWORD dwWaitTime = INFINITE);

    //�߳�ִ������
    virtual bool OnThreadEventStart() { return true; };
    virtual bool OnThreadEventRun();
    virtual bool OnThreadEventEnd() { return true; };
    virtual bool OnHandleMsg(MSG msg){ return true; };

    BOOL PostThreadMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    //�̻߳ص�����
    static unsigned __stdcall myThreadFunc(LPVOID lpParam);
private:
    HANDLE m_hThreadHandle;
    DWORD m_ThreadID;
};

