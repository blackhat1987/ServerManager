#pragma once
#include <afxtempl.h>
#include <cstring>
#include "../Common/Buffer.h"
#include "../Common/DataStruct.h"

#define CLIENT_RECVBUFSIZE 1024 * 8
#define PACKET_MAX_SIZE 16384

struct tagClientContext
{
    SOCKET          s;                          //客户端套接字
    sockaddr_in     addr;                       //客户端套接字信息

    CBuffer         m_SendBuffer;               //发送缓冲区
    CBuffer         m_UncompressBuffer;         //压缩后的缓冲区
    CBuffer         m_CompressBuffer;           //解压后的缓冲区

    HANDLE          m_hWriteEvent;
    DWORD           dwLastTime;                 //心跳计时

    WSABUF			m_wsaInBuffer;              //接收重叠缓冲区
    BYTE			m_byInBuffer[8192];         //接收重叠缓冲区   跟m_wsaInBuffer共用
    WSABUF			m_wsaOutBuffer;             //发送重叠缓冲区

    DWORD           m_Dialog[2];                // 放对话框列表用，第一个int是类型，第二个是CDialog的地址
    bool            m_Ismain;                   //判断是不是主线程
};
typedef struct tagClientContext CLIENT_CONTEXT;
typedef struct tagClientContext *PCLIENT_CONTEXT;

class CClientManager
{
public:
    CClientManager(CRITICAL_SECTION* cs);
    ~CClientManager();

    void AddClient(SOCKET s, CLIENT_CONTEXT* pClientContext);
    void DeletClient(SOCKET s);
    void RemoveClient();
    PCLIENT_CONTEXT GetClient(SOCKET s);


    CMap<SOCKET, SOCKET, CLIENT_CONTEXT*, CLIENT_CONTEXT*> m_ClientMap;
private:
    
    CRITICAL_SECTION* m_cs;
};

