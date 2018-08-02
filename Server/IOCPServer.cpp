#include "stdafx.h"
#include "IOCPServer.h"


CIOCPServer::CIOCPServer()
{
    //�ֶ����� ��ʼ��Ϊ���ź� 
    m_hKillEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    m_pNotifyProc = NULL;
    m_nWorkerCnt = 0;
    m_nSendKbps = 0;
    m_nRecvKbps = 0;
    m_pClientManager = NULL;   
}


CIOCPServer::~CIOCPServer()
{
    OnClose();
}


unsigned CIOCPServer::ListenPorc(LPVOID lpParam)
{
    CIOCPServer* pThis = (CIOCPServer*)lpParam;
    while (true)
    {
        //���߳̽����ź�
        if (WaitForSingleObject(pThis->m_hKillEvent, 100) == WAIT_OBJECT_0)
            break;
        DWORD dwRet;
        //�ȴ��첽�¼�
        dwRet = WSAWaitForMultipleEvents(1,&pThis->m_hEvent,FALSE,100,FALSE);
        //��ʱ������һ��
        if (dwRet == WSA_WAIT_TIMEOUT)
            continue;
        WSANETWORKEVENTS wsaEvnets;
        int nRet = WSAEnumNetworkEvents(pThis->m_ServerSocket,pThis->m_hEvent,&wsaEvnets);
        if (nRet == SOCKET_ERROR)
        {
            return false;
        } 
        if (wsaEvnets.lNetworkEvents & FD_ACCEPT && wsaEvnets.iErrorCode[FD_ACCEPT_BIT] == 0)
        {
            OutputDebugStringA("Log:������������");
            pThis->OnAccept();
        }
    }
    OutputDebugStringA("Log:�����߳��˳�");
    return false;
}

bool CIOCPServer::Initialize(NOTIFYPROC pNotifyProc, int nMaxConnections, int nPort)
{
    //������ص�����ָ��
    m_pNotifyProc = pNotifyProc;
    m_nMaxConnections = nMaxConnections;
    //�����ص��׽���
    m_ServerSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (m_ServerSocket == INVALID_SOCKET)
    {
        OutputDebugStringA("Log:�����׽���ʧ��");
        return false;
    }
    //�����¼�
    m_hEvent = WSACreateEvent();
    if (m_hEvent == WSA_INVALID_EVENT)
    {
        OutputDebugStringA("Log:�����¼�ʧ��");
        closesocket(m_ServerSocket);
        return false;
    }
    //���׽����¼�
    int nRet = WSAEventSelect(m_ServerSocket, m_hEvent, FD_ACCEPT); 
    if (nRet == SOCKET_ERROR)
    {
        OutputDebugStringA("Log:���¼�ʧ��");
        closesocket(m_ServerSocket);
        return false;
    }
    //��д��ַ
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ADDR_ANY;
    addr.sin_port = htons(nPort);
    //��IP�Ͷ˿�
    nRet = bind(m_ServerSocket, (sockaddr*)&addr, sizeof(sockaddr));
    if (nRet == SOCKET_ERROR)
    {
        OutputDebugStringA("Log:���׽���ʧ��");
        closesocket(m_ServerSocket);
        return false;
    }
    //��ʼ����
    nRet = listen(m_ServerSocket, SOMAXCONN);
    if (nRet == SOCKET_ERROR)
    {
        OutputDebugStringA("Log:���׽���ʧ��");
        closesocket(m_ServerSocket);
        return false;
    }
    //���������߳�
    m_ListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenPorc, this, 0, NULL);
    //������������߳�
    m_SocketCheckThread = (HANDLE)_beginthreadex(NULL, 0, CheckPorc, this, 0, NULL);
    if (m_ListenThread != INVALID_HANDLE_VALUE && m_SocketCheckThread != INVALID_HANDLE_VALUE)
    {
        //��ʼ��IOCP
        InitializeIOCP();
        OutputDebugStringA("Log:��ʼ��IOCP���");
        return true;
    }
    return false;
}

unsigned CIOCPServer::CheckPorc(LPVOID lpParam)
{
    CIOCPServer* pThis = (CIOCPServer*)lpParam;
    while (true)
    {
        //�ȴ��߳��˳��ź�
        if (WaitForSingleObject(pThis->m_hKillEvent, 100) == WAIT_OBJECT_0)
            break;
        OutputDebugStringA("Log:��ʼ�������");
        //10����һ��
        Sleep(1000);
        pThis->SocketCheck();
    }
    OutputDebugStringA("Log:��������߳��˳�");
    return false;
}
unsigned CIOCPServer::ThreadPoolFunc(LPVOID lpParam)
{
    CIOCPServer* pThis = (CIOCPServer*)lpParam;
    BOOL bRet = FALSE;
    ULONG ulFlags = MSG_PARTIAL;
    DWORD dwTransferedBytes = 0;
    tagClientContext* pClientContext = NULL;
    LPOVERLAPPED lpOverlapped = NULL;
    tagMyOVERLAPPED* pMyOVERLAPPED = NULL;
    while (true)
    {
        //����ɶ˿�ȡ��Ϣ
        bRet = GetQueuedCompletionStatus(pThis->m_hCompletionPort, &dwTransferedBytes, (LPDWORD)&pClientContext, &lpOverlapped, INFINITE);
        //�����ص��ṹƫ��
        pMyOVERLAPPED = CONTAINING_RECORD(lpOverlapped, tagMyOVERLAPPED, m_ol);
        if (!bRet)
        {
            //û����Ϣ ��һ��
            continue;
        }
        if (pMyOVERLAPPED ==NULL)
        {
            return false;
        }
        //��ʼ������Ϣ����
        switch (pMyOVERLAPPED->m_type)
        {
        case IOInitialize:
            break;
        case IORead:
            pThis->OnRecv(pClientContext, dwTransferedBytes);
            break;
        case IOWrite:
            pThis->OnSend(pClientContext, dwTransferedBytes);
            break;
        default:
            break;
        }

    }
    InterlockedDecrement(&pThis->m_nWorkerCnt);//�߳��˳���һ
    if (pMyOVERLAPPED)//�߳��˳����ͷ���Դ
    {
        delete pMyOVERLAPPED;
    }
    return true;
}
bool CIOCPServer::InitializeIOCP()
{
    SOCKET s;
    SYSTEM_INFO systemInfo;
    //�����׽���
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (s == INVALID_SOCKET)
    {
        return false;
    }
    //������ɶ˿�
    m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (m_hCompletionPort == NULL)
    {
        closesocket(s);
        return false;
    }

    //��ȡCPU������
    GetSystemInfo(&systemInfo);
    int nThreadCount = systemInfo.dwNumberOfProcessors;

    HANDLE hWorkThread = INVALID_HANDLE_VALUE;
    for (int i = 0; i < nThreadCount;i++)
    {
        hWorkThread = (HANDLE)_beginthreadex(NULL, 0, ThreadPoolFunc, this, 0, NULL);
        if (hWorkThread == INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_hCompletionPort);
            return false;
        }
        //����һ���߳�+1
        InterlockedIncrement(&m_nWorkerCnt);
        CloseHandle(hWorkThread);//��ǰ�ͷ���Դ,�̲߳����˳�
    }
    //����Socket�������
    m_pClientManager = new CClientManager(&cs);
    if (m_pClientManager == NULL)
    {
        closesocket(s);
        CloseHandle(m_hCompletionPort);
        return false;
    }
    return true;
}

bool CIOCPServer::OnAccept()
{
    SOCKADDR_IN ClientAddr;
    ClientAddr.sin_family = AF_INET;
    SOCKET ClientSock = INVALID_SOCKET;
    int nLen = sizeof(SOCKADDR_IN);

    ClientSock = accept(m_ServerSocket, (sockaddr*)&ClientAddr, &nLen);
    if (ClientSock == INVALID_SOCKET)
    {
        return false;
    }
    tagClientContext* pClientContext = new tagClientContext;
    if (pClientContext == NULL)
    {
        return false;
    }
    //��ʼ��
    pClientContext->s = ClientSock;
    pClientContext->addr = ClientAddr;
    pClientContext->dwLastTime = GetTickCount() /1000;
    pClientContext->m_Ismain = false;
    memset(pClientContext->m_Dialog, 0, sizeof(pClientContext->m_Dialog));
    //ӳ�仺���� ʵ�ֹ���
    pClientContext->m_wsaInBuffer.buf = (char*)pClientContext->m_byInBuffer;
    pClientContext->m_wsaInBuffer.len = sizeof(pClientContext->m_byInBuffer);
    //���׽��ֵ���ɶ˿�
    if (!Associated(ClientSock, (DWORD)pClientContext))
    {
        delete pClientContext;
        pClientContext = NULL;
        closesocket(ClientSock);
        closesocket(m_ServerSocket);
        return false;
    }
    //��ӵ�hash��
    m_pClientManager->AddClient(ClientSock, pClientContext);

    tagMyOVERLAPPED*  pMyOVERLAPPED = new tagMyOVERLAPPED;
    if (pMyOVERLAPPED == NULL)
    {
        return false;
    }
    pMyOVERLAPPED->m_type = IOInitialize;
    //Ͷ�ݳ�ʼ��������ɶ˿�
    BOOL bSuccess =  PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD)pClientContext, &pMyOVERLAPPED->m_ol);
    if ((!bSuccess && GetLastError() != ERROR_IO_PENDING))
    {
        return false;
    }
    //֪ͨ�����������ӽ���
    m_pNotifyProc(NULL, pClientContext, NC_CLIENT_CONNECT);
    //Ͷ��һ������������ɶ˿�
    PostRecv(pClientContext);
    return true;
}

bool CIOCPServer::Associated(SOCKET s, DWORD dwCompletionKey)
{
    HANDLE hIOCP = CreateIoCompletionPort((HANDLE)s,m_hCompletionPort,dwCompletionKey,0);
    return hIOCP != NULL ? true : false;
}

bool CIOCPServer::PostRecv(tagClientContext* pClientContext)
{
    tagMyOVERLAPPED*  pMyOVERLAPPED = new tagMyOVERLAPPED;
    if (pMyOVERLAPPED == NULL)
    {
        return false;
    }
    DWORD dwRecvedBytes;
    pMyOVERLAPPED->m_type = IORead;
    ULONG ulFlags = MSG_PARTIAL;
    //Ͷ�ݽ�������
    int nRet = WSARecv(pClientContext->s, &pClientContext->m_wsaInBuffer, 1, &dwRecvedBytes, &ulFlags,
        (WSAOVERLAPPED*)&pMyOVERLAPPED->m_ol, NULL);
    if (nRet == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
    {
        return FALSE;
    }
    return TRUE;
}

bool CIOCPServer::OnRecv(tagClientContext* pClientContext, DWORD dwTransferedBytes)
{
    /////////////////////////////�����ٶ�////////////////////////////////
    static DWORD nLastTick = GetTickCount();
    static DWORD nBytes = 0;
    nBytes += dwTransferedBytes;

    if (GetTickCount() - nLastTick >= 1000)
    {
        nLastTick = GetTickCount()/1000;
        InterlockedExchange((LPLONG)&(m_nRecvKbps), nBytes);
        nBytes = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    if (dwTransferedBytes == 0)
    {
        //�ͻ��˶Ͽ�����
        EnterCriticalSection(m_cs);
        closesocket(pClientContext->s);
        m_pClientManager->DeletClient(pClientContext->s);//�Ӷ�����T��
        m_pNotifyProc(NULL, pClientContext, NC_CLIENT_DISCONNECT);
        delete pClientContext;
        LeaveCriticalSection(m_cs);
        return false;
    }
    //���ص��������ж�ȡ���ݵ�ѹ��������
    pClientContext->m_UncompressBuffer.Write(pClientContext->m_byInBuffer, dwTransferedBytes);
    m_pNotifyProc(NULL, pClientContext, NC_RECEIVE);//�յ���Ϣ����������
    while (pClientContext->m_UncompressBuffer.GetBufferLen() > sizeof(tagPacketHead))
    {
        //���������������֤
        pClientContext->dwLastTime = GetTickCount() / 1000;
        //���ݽ�ѹ����
        tagPacketHead pPacketHead = { 0 };
        //��ȡ���ݳ���
        memcpy(&pPacketHead, pClientContext->m_UncompressBuffer.GetBuffer(0), sizeof(tagPacketHead));
        //�ж������Ƿ����  ���ܳ���
        if (pPacketHead.m_dwSize >0 && pClientContext->m_UncompressBuffer.GetBufferLen() >= pPacketHead.m_dwSize)
        {
            //һ��Ҫ��ʼ��
            uLong nUnCompressLength = 0;
            //������-��ͷ = ���ݳ���
            int nSize = pPacketHead.m_dwSize - sizeof(tagPacketHead);
            //�����ѹ���ݳ���
            nUnCompressLength = pPacketHead.m_dwUnSize;
            //�����ѹ������
            PBYTE pData = new BYTE[nSize];
            if (pData == NULL)
            {
                //û���뵽�ռ䴦��
                return false;
            }
            //�����ѹ��Ŀռ�
            PBYTE pBuf = new BYTE[nUnCompressLength];
            if (pBuf == NULL)
            {
                //û���뵽�ռ䴦��
                return false;
            }
            memset(pData, 0, nSize);
            memset(pBuf, 0, nUnCompressLength);
            //�������ͷ
            pClientContext->m_UncompressBuffer.Delete(sizeof(tagPacketHead));
            //��Ҫʣ�����ݸ��Ƶ���ѹ������
            pClientContext->m_UncompressBuffer.Read(pData, nSize);
            //��ʼ��ѹ
            if (uncompress((LPBYTE)pBuf, &nUnCompressLength, (LPBYTE)pData, nSize) != Z_OK)
            {
                delete[] pData;
                delete[] pBuf;
                OutputDebugStringA("Log:��ѹ����ʧ��!");
                return false;
            }
            //����ѹ������ݸ��Ƶ�������
            pClientContext->m_CompressBuffer.ClearBuffer();
            pClientContext->m_CompressBuffer.Write(pBuf, nUnCompressLength);
            //֪ͨ������
            m_pNotifyProc(NULL, pClientContext, NC_RECEIVE_COMPLETE);
            delete[] pData;
            delete[] pBuf;
        }
        else
        {
            break;
        }
    }
    PostRecv(pClientContext);
    return true;
}

bool CIOCPServer::OnSend(tagClientContext* pClientContext, DWORD dwTransferedBytes)
{
    /////////////////////////�����ٶ�/////////////////////////////////
    static DWORD nLastTick = GetTickCount();
    static DWORD nBytes = 0;

    nBytes += dwTransferedBytes;

    if (GetTickCount() - nLastTick >= 1000)
    {
        nLastTick = GetTickCount()/1000;
        InterlockedExchange((LPLONG)&(m_nSendKbps), nBytes);
        nBytes = 0;
    }
    //////////////////////////////////////////////////////////////////////////
    //���Ͷ��پ���ն���
    m_cs.Lock();
    pClientContext->m_SendBuffer.Delete(dwTransferedBytes);
    m_cs.Unlock();
    if (pClientContext->m_SendBuffer.GetBufferLen() == 0)
    {
        //��ջ�����
        pClientContext->m_SendBuffer.ClearBuffer(); 
        //�����ź��¼�  ��ֹͬʱд������
        SetEvent(pClientContext->m_hWriteEvent);    
        return true;
    }
    else
    {
        m_pNotifyProc(NULL, pClientContext, NC_TRANSMIT);//�������������ݻ��ڷ���

        tagMyOVERLAPPED*  pMyOVERLAPPED = new tagMyOVERLAPPED;
        if (pMyOVERLAPPED == NULL)
        {
            return false;
        }
        //����Ͷ��
        ULONG ulFlags = MSG_PARTIAL;
        pMyOVERLAPPED->m_type = IOWrite;
        pClientContext->m_wsaOutBuffer.buf = (char*)pClientContext->m_SendBuffer.GetBuffer();
        pClientContext->m_wsaOutBuffer.len = pClientContext->m_SendBuffer.GetBufferLen();
        int nRet = WSASend(pClientContext->s, &pClientContext->m_wsaOutBuffer, 1,
            &pClientContext->m_wsaOutBuffer.len, ulFlags, (WSAOVERLAPPED*)&pMyOVERLAPPED->m_ol, NULL);
        if (nRet == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
        {
            return false;
        }
    }
    return true;
}


bool CIOCPServer::Send(tagClientContext* pClientContext, char* pBuffer, int nSize)
{
    if (pClientContext == NULL)
    {
        return false;
    }
    if (nSize > 0)
    {
        //��ʼѹ��
        //����ѹ���󻺳�����С
        uLong nUnSize = compressBound(nSize);
        PBYTE pData = new BYTE[nUnSize];
        if (pData == NULL)
        {
            return false;
        }
        memset(pData, 0, nUnSize);
        //��ʼѹ��
        if (compress((LPBYTE)pData, &nUnSize, (LPBYTE)pBuffer, nSize) != Z_OK)
        {
            OutputDebugStringA("Log:compress failed!\n");
            delete[] pData;
            return false;
        }
        //���������= ѹ����ĳ��� + ��ͷ����
        int nPacketLen = nUnSize + sizeof(tagPacketHead);
        //д�������
        pClientContext->m_SendBuffer.Write((PBYTE)&nPacketLen, sizeof(DWORD));
        //д���ѹ��ĳ���
        pClientContext->m_SendBuffer.Write((PBYTE)&nSize, sizeof(DWORD));
        //д�����ݵ�������
        pClientContext->m_SendBuffer.Write((LPBYTE)pData, nUnSize);
        delete[] pData;
    }
    tagMyOVERLAPPED*  pMyOVERLAPPED = new tagMyOVERLAPPED;
    pMyOVERLAPPED->m_type = IOWrite;
    PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD)pClientContext, &pMyOVERLAPPED->m_ol);
    return true;
}

//�����������ܵ��� ������ͬ��
void CIOCPServer::CloseCompletionPort()
{
    while (m_nWorkerCnt)
    {
        PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD)NULL, NULL);
        Sleep(100);
        InterlockedDecrement(&m_nWorkerCnt);//�߳��˳���һ
    }
    POSITION pos = m_pClientManager->m_ClientMap.GetStartPosition();
    while (pos != NULL)
    {
        tagClientContext* pClientContext = NULL;
        SOCKET temp;
        m_pClientManager->m_ClientMap.GetNextAssoc(pos, temp, pClientContext);
        closesocket(temp);
        m_pNotifyProc(NULL, pClientContext, NC_CLIENT_DISCONNECT);
        //�Ӷ�����T��
        m_pClientManager->DeletClient(temp);
        delete pClientContext;
    }
}

bool CIOCPServer::OnClose()
{
    SetEvent(m_hKillEvent);//�����¼�,�����߳��˳�
    WaitForSingleObject(m_ListenThread, INFINITE); 
    WaitForSingleObject(m_SocketCheckThread, INFINITE);
    CloseHandle(m_ListenThread);
    CloseHandle(m_SocketCheckThread);
    CloseHandle(m_hKillEvent);  
    closesocket(m_ServerSocket);
    WSACloseEvent(m_hEvent);
    CloseCompletionPort();
    if (m_pClientManager != NULL)
    {
        delete m_pClientManager;
        m_pClientManager = NULL;
    }
    return true;
}

//�������
void CIOCPServer::SocketCheck()
{
    if (m_pClientManager->m_ClientMap.GetCount() < 1)
    {
        Dbgprintf("Log:û����������");
        return;
    }
    EnterCriticalSection(m_cs);
    POSITION pos = m_pClientManager->m_ClientMap.GetStartPosition();
    while (pos != NULL)
    {
        tagClientContext* pClientContext = NULL;
        SOCKET s;
        m_pClientManager->m_ClientMap.GetNextAssoc(pos, s, pClientContext);
        if (pClientContext->m_Ismain)
        {
            long chazhi = GetTickCount() / 1000 - pClientContext->dwLastTime;
            Dbgprintf("��ǰ���ʱ��%d", GetTickCount() / 1000);
            if (chazhi > timeout)
            {
                Dbgprintf("������ʱ%d", chazhi);
                closesocket(s);
                m_pClientManager->DeletClient(s);//�Ӷ�����T��
                m_pNotifyProc(NULL, pClientContext, NC_CLIENT_DISCONNECT);
                delete pClientContext;
            }
        }
    }
    LeaveCriticalSection(m_cs);
}
