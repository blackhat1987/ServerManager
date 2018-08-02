#pragma once
#include <afxtempl.h>
#include <cstring>
#include "../Common/Buffer.h"
#include "../Common/DataStruct.h"

#define CLIENT_RECVBUFSIZE 1024 * 8
#define PACKET_MAX_SIZE 16384

struct tagClientContext
{
    SOCKET          s;                          //�ͻ����׽���
    sockaddr_in     addr;                       //�ͻ����׽�����Ϣ

    CBuffer         m_SendBuffer;               //���ͻ�����
    CBuffer         m_UncompressBuffer;         //ѹ����Ļ�����
    CBuffer         m_CompressBuffer;           //��ѹ��Ļ�����

    HANDLE          m_hWriteEvent;
    DWORD           dwLastTime;                 //������ʱ

    WSABUF			m_wsaInBuffer;              //�����ص�������
    BYTE			m_byInBuffer[8192];         //�����ص�������   ��m_wsaInBuffer����
    WSABUF			m_wsaOutBuffer;             //�����ص�������

    DWORD           m_Dialog[2];                // �ŶԻ����б��ã���һ��int�����ͣ��ڶ�����CDialog�ĵ�ַ
    bool            m_Ismain;                   //�ж��ǲ������߳�
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

