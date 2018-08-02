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

    static unsigned __stdcall ListenPorc(LPVOID lpParam);   //�����������߳�
    static unsigned __stdcall ThreadPoolFunc(LPVOID lpParam);  //�����̳߳�
    static unsigned __stdcall CheckPorc(LPVOID lpParam);    //��������߳�

    bool Initialize(NOTIFYPROC pNotifyProc,int nMaxConnections, int nPort); //�ص�������
    bool Associated(SOCKET s, DWORD dwCompletionKey);   //����ɶ˿�
    bool OnAccept();                                    //������������
    bool PostRecv(tagClientContext* pClientContext);                           //�����������
    bool Send(tagClientContext* pClientContext,char* pbuf, int nSize);        //��������
    bool OnRecv(tagClientContext* pClientContext, DWORD dwTransferedBytes);   //�ӻ�������ȡ����
    bool OnSend(tagClientContext* pClientContext, DWORD dwTransferedBytes);   //д���ݵ�������
    bool OnClose();                                         //�رշ����
    void CloseCompletionPort();                             //�ر���ɶ˿�


    void SocketCheck();                                     //������⺯��
    CClientManager*     m_pClientManager;                    //sock�������

protected:
    CInitSock            m_InitSock;                      //��ʼ���׽������
    SOCKET               m_ServerSocket;
    NOTIFYPROC			 m_pNotifyProc;                    //�ص����� 
    DWORD                m_nMaxConnections;                 //���������

    WSAEVENT             m_hEvent;                         //�첽�¼����
    HANDLE               m_hKillEvent;                      //�߳��˳��¼�

    HANDLE               m_ListenThread;                   //�����߳̾��
    HANDLE               m_hCompletionPort;                //��ɶ˿ڵľ��
    HANDLE               m_SocketCheckThread;               //��������߳�

    CCriticalSection      m_cs;                            //��ʼ���ٽ���
    LONG                  m_nWorkerCnt;                      //��¼�߳���
    

    UINT					m_nSendKbps;                  // ���ͼ�ʱ�ٶ�
    UINT					m_nRecvKbps;                  // ���ܼ�ʱ�ٶ�   

    CRITICAL_SECTION        cs;                             //��socket���������
};

