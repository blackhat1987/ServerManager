#pragma once
#include "../Common/InitSock.h"
#include "../Common/ClientManager.h"
#include "../Common/DataStruct.h"
#include "../Common/zlib.h"
#ifdef _DEBUG
#pragma comment(lib,"..//Common//ZlibStatDebug//zlibstat.lib")
#else
#pragma comment(lib,"..//Common//ZlibStatRelease//zlibstat.lib")
#endif // DEBUG

typedef void (CALLBACK* NOTIFYPROC)(LPVOID, CLIENT_CONTEXT*, UINT nCode);

enum emIOCP_TYPE
{
    IOInitialize,
    IORead,
    IOWrite
};

struct tagMyOVERLAPPED
{
    WSAOVERLAPPED m_ol;
    emIOCP_TYPE  m_type;
    tagMyOVERLAPPED()
    {
        memset(&m_ol, 0, sizeof(tagMyOVERLAPPED));
    }

};

class CIOCPServer
{
public:
    CIOCPServer();
    ~CIOCPServer();
    bool InitializeIOCP();

    static unsigned __stdcall ListenPorc(LPVOID lpParam);   //服务器监听线程
    static unsigned __stdcall ThreadPoolFunc(LPVOID lpParam);  //工作线程池
    static unsigned __stdcall CheckPorc(LPVOID lpParam);    //心跳检测线程

    bool Initialize(NOTIFYPROC pNotifyProc,int nMaxConnections, int nPort); //回调函数用
    bool Associated(SOCKET s, DWORD dwCompletionKey);   //绑定完成端口
    bool OnAccept();                                    //处理连接请求
    bool PostRecv(tagClientContext* pClientContext);                           //处理接受请求
    bool Send(tagClientContext* pClientContext,char* pbuf, int nSize);        //发送数据
    bool OnRecv(tagClientContext* pClientContext, DWORD dwTransferedBytes);   //从缓冲区读取数据
    bool OnSend(tagClientContext* pClientContext, DWORD dwTransferedBytes);   //写数据到缓冲区
    bool OnClose();                                         //关闭服务端
    void CloseCompletionPort();                             //关闭完成端口


    void SocketCheck();                                     //心跳检测函数
    CClientManager*     m_pClientManager;                    //sock管理对象

protected:
    CInitSock            m_InitSock;                      //初始化套接字组件
    SOCKET               m_ServerSocket;
    NOTIFYPROC			 m_pNotifyProc;                    //回调函数 
    DWORD                m_nMaxConnections;                 //最大上线数

    WSAEVENT             m_hEvent;                         //异步事件句柄
    HANDLE               m_hKillEvent;                      //线程退出事件

    HANDLE               m_ListenThread;                   //监听线程句柄
    HANDLE               m_hCompletionPort;                //完成端口的句柄
    HANDLE               m_SocketCheckThread;               //心跳检测线程

    CCriticalSection      m_cs;                            //初始化临界区
    LONG                  m_nWorkerCnt;                      //记录线程数
    

    UINT					m_nSendKbps;                  // 发送即时速度
    UINT					m_nRecvKbps;                  // 接受即时速度   

    CRITICAL_SECTION        cs;                             //给socket管理对象用
};

