#include "stdafx.h"
#include "IOCPServer.h"


CIOCPServer::CIOCPServer()
{
    //手动重置 初始化为无信号 
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
        //等线程结束信号
        if (WaitForSingleObject(pThis->m_hKillEvent, 100) == WAIT_OBJECT_0)
            break;
        DWORD dwRet;
        //等待异步事件
        dwRet = WSAWaitForMultipleEvents(1,&pThis->m_hEvent,FALSE,100,FALSE);
        //超时进入下一轮
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
            OutputDebugStringA("Log:处理连接请求");
            pThis->OnAccept();
        }
    }
    OutputDebugStringA("Log:监听线程退出");
    return false;
}

bool CIOCPServer::Initialize(NOTIFYPROC pNotifyProc, int nMaxConnections, int nPort)
{
    //主程序回调函数指针
    m_pNotifyProc = pNotifyProc;
    m_nMaxConnections = nMaxConnections;
    //创建重叠套接字
    m_ServerSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (m_ServerSocket == INVALID_SOCKET)
    {
        OutputDebugStringA("Log:创建套接字失败");
        return false;
    }
    //创建事件
    m_hEvent = WSACreateEvent();
    if (m_hEvent == WSA_INVALID_EVENT)
    {
        OutputDebugStringA("Log:创建事件失败");
        closesocket(m_ServerSocket);
        return false;
    }
    //绑定套接字事件
    int nRet = WSAEventSelect(m_ServerSocket, m_hEvent, FD_ACCEPT); 
    if (nRet == SOCKET_ERROR)
    {
        OutputDebugStringA("Log:绑定事件失败");
        closesocket(m_ServerSocket);
        return false;
    }
    //填写地址
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ADDR_ANY;
    addr.sin_port = htons(nPort);
    //绑定IP和端口
    nRet = bind(m_ServerSocket, (sockaddr*)&addr, sizeof(sockaddr));
    if (nRet == SOCKET_ERROR)
    {
        OutputDebugStringA("Log:绑定套接字失败");
        closesocket(m_ServerSocket);
        return false;
    }
    //开始监听
    nRet = listen(m_ServerSocket, SOMAXCONN);
    if (nRet == SOCKET_ERROR)
    {
        OutputDebugStringA("Log:绑定套接字失败");
        closesocket(m_ServerSocket);
        return false;
    }
    //创建监听线程
    m_ListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenPorc, this, 0, NULL);
    //创建心跳检测线程
    m_SocketCheckThread = (HANDLE)_beginthreadex(NULL, 0, CheckPorc, this, 0, NULL);
    if (m_ListenThread != INVALID_HANDLE_VALUE && m_SocketCheckThread != INVALID_HANDLE_VALUE)
    {
        //初始化IOCP
        InitializeIOCP();
        OutputDebugStringA("Log:初始化IOCP完成");
        return true;
    }
    return false;
}

unsigned CIOCPServer::CheckPorc(LPVOID lpParam)
{
    CIOCPServer* pThis = (CIOCPServer*)lpParam;
    while (true)
    {
        //等待线程退出信号
        if (WaitForSingleObject(pThis->m_hKillEvent, 100) == WAIT_OBJECT_0)
            break;
        OutputDebugStringA("Log:开始心跳检测");
        //10秒检测一次
        Sleep(1000);
        pThis->SocketCheck();
    }
    OutputDebugStringA("Log:心跳检测线程退出");
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
        //从完成端口取消息
        bRet = GetQueuedCompletionStatus(pThis->m_hCompletionPort, &dwTransferedBytes, (LPDWORD)&pClientContext, &lpOverlapped, INFINITE);
        //计算重叠结构偏移
        pMyOVERLAPPED = CONTAINING_RECORD(lpOverlapped, tagMyOVERLAPPED, m_ol);
        if (!bRet)
        {
            //没有消息 下一轮
            continue;
        }
        if (pMyOVERLAPPED ==NULL)
        {
            return false;
        }
        //开始处理消息类型
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
    InterlockedDecrement(&pThis->m_nWorkerCnt);//线程退出减一
    if (pMyOVERLAPPED)//线程退出就释放资源
    {
        delete pMyOVERLAPPED;
    }
    return true;
}
bool CIOCPServer::InitializeIOCP()
{
    SOCKET s;
    SYSTEM_INFO systemInfo;
    //创建套接字
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (s == INVALID_SOCKET)
    {
        return false;
    }
    //创建完成端口
    m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (m_hCompletionPort == NULL)
    {
        closesocket(s);
        return false;
    }

    //获取CPU核心数
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
        //创建一个线程+1
        InterlockedIncrement(&m_nWorkerCnt);
        CloseHandle(hWorkThread);//提前释放资源,线程不会退出
    }
    //创建Socket管理对象
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
    //初始化
    pClientContext->s = ClientSock;
    pClientContext->addr = ClientAddr;
    pClientContext->dwLastTime = GetTickCount() /1000;
    pClientContext->m_Ismain = false;
    memset(pClientContext->m_Dialog, 0, sizeof(pClientContext->m_Dialog));
    //映射缓存区 实现共用
    pClientContext->m_wsaInBuffer.buf = (char*)pClientContext->m_byInBuffer;
    pClientContext->m_wsaInBuffer.len = sizeof(pClientContext->m_byInBuffer);
    //绑定套接字到完成端口
    if (!Associated(ClientSock, (DWORD)pClientContext))
    {
        delete pClientContext;
        pClientContext = NULL;
        closesocket(ClientSock);
        closesocket(m_ServerSocket);
        return false;
    }
    //添加到hash表
    m_pClientManager->AddClient(ClientSock, pClientContext);

    tagMyOVERLAPPED*  pMyOVERLAPPED = new tagMyOVERLAPPED;
    if (pMyOVERLAPPED == NULL)
    {
        return false;
    }
    pMyOVERLAPPED->m_type = IOInitialize;
    //投递初始化请求到完成端口
    BOOL bSuccess =  PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD)pClientContext, &pMyOVERLAPPED->m_ol);
    if ((!bSuccess && GetLastError() != ERROR_IO_PENDING))
    {
        return false;
    }
    //通知主程序有连接进来
    m_pNotifyProc(NULL, pClientContext, NC_CLIENT_CONNECT);
    //投递一个接收请求到完成端口
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
    //投递接收请求
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
    /////////////////////////////接收速度////////////////////////////////
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
        //客户端断开连接
        EnterCriticalSection(m_cs);
        closesocket(pClientContext->s);
        m_pClientManager->DeletClient(pClientContext->s);//从队列中T出
        m_pNotifyProc(NULL, pClientContext, NC_CLIENT_DISCONNECT);
        delete pClientContext;
        LeaveCriticalSection(m_cs);
        return false;
    }
    //从重叠缓冲区中读取数据到压缩缓冲区
    pClientContext->m_UncompressBuffer.Write(pClientContext->m_byInBuffer, dwTransferedBytes);
    m_pNotifyProc(NULL, pClientContext, NC_RECEIVE);//收到消息告诉主程序
    while (pClientContext->m_UncompressBuffer.GetBufferLen() > sizeof(tagPacketHead))
    {
        //这里可以做数据验证
        pClientContext->dwLastTime = GetTickCount() / 1000;
        //数据解压处理
        tagPacketHead pPacketHead = { 0 };
        //读取数据长度
        memcpy(&pPacketHead, pClientContext->m_UncompressBuffer.GetBuffer(0), sizeof(tagPacketHead));
        //判断数据是否完成  包总长度
        if (pPacketHead.m_dwSize >0 && pClientContext->m_UncompressBuffer.GetBufferLen() >= pPacketHead.m_dwSize)
        {
            //一定要初始化
            uLong nUnCompressLength = 0;
            //包长度-包头 = 数据长度
            int nSize = pPacketHead.m_dwSize - sizeof(tagPacketHead);
            //计算解压数据长度
            nUnCompressLength = pPacketHead.m_dwUnSize;
            //申请解压缓冲区
            PBYTE pData = new BYTE[nSize];
            if (pData == NULL)
            {
                //没申请到空间处理
                return false;
            }
            //申请解压后的空间
            PBYTE pBuf = new BYTE[nUnCompressLength];
            if (pBuf == NULL)
            {
                //没申请到空间处理
                return false;
            }
            memset(pData, 0, nSize);
            memset(pBuf, 0, nUnCompressLength);
            //清空垃圾头
            pClientContext->m_UncompressBuffer.Delete(sizeof(tagPacketHead));
            //将要剩余数据复制到解压缓冲区
            pClientContext->m_UncompressBuffer.Read(pData, nSize);
            //开始解压
            if (uncompress((LPBYTE)pBuf, &nUnCompressLength, (LPBYTE)pData, nSize) != Z_OK)
            {
                delete[] pData;
                delete[] pBuf;
                OutputDebugStringA("Log:解压数据失败!");
                return false;
            }
            //将解压后的数据复制到缓冲区
            pClientContext->m_CompressBuffer.ClearBuffer();
            pClientContext->m_CompressBuffer.Write(pBuf, nUnCompressLength);
            //通知主程序
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
    /////////////////////////发送速度/////////////////////////////////
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
    //发送多少就清空多少
    m_cs.Lock();
    pClientContext->m_SendBuffer.Delete(dwTransferedBytes);
    m_cs.Unlock();
    if (pClientContext->m_SendBuffer.GetBufferLen() == 0)
    {
        //清空缓冲区
        pClientContext->m_SendBuffer.ClearBuffer(); 
        //重置信号事件  防止同时写缓冲区
        SetEvent(pClientContext->m_hWriteEvent);    
        return true;
    }
    else
    {
        m_pNotifyProc(NULL, pClientContext, NC_TRANSMIT);//告诉主程序数据还在发送

        tagMyOVERLAPPED*  pMyOVERLAPPED = new tagMyOVERLAPPED;
        if (pMyOVERLAPPED == NULL)
        {
            return false;
        }
        //重新投递
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
        //开始压缩
        //计算压缩后缓冲区大小
        uLong nUnSize = compressBound(nSize);
        PBYTE pData = new BYTE[nUnSize];
        if (pData == NULL)
        {
            return false;
        }
        memset(pData, 0, nUnSize);
        //开始压缩
        if (compress((LPBYTE)pData, &nUnSize, (LPBYTE)pBuffer, nSize) != Z_OK)
        {
            OutputDebugStringA("Log:compress failed!\n");
            delete[] pData;
            return false;
        }
        //计算包长度= 压缩后的长度 + 包头长度
        int nPacketLen = nUnSize + sizeof(tagPacketHead);
        //写入包长度
        pClientContext->m_SendBuffer.Write((PBYTE)&nPacketLen, sizeof(DWORD));
        //写入解压后的长度
        pClientContext->m_SendBuffer.Write((PBYTE)&nSize, sizeof(DWORD));
        //写入数据到缓冲区
        pClientContext->m_SendBuffer.Write((LPBYTE)pData, nUnSize);
        delete[] pData;
    }
    tagMyOVERLAPPED*  pMyOVERLAPPED = new tagMyOVERLAPPED;
    pMyOVERLAPPED->m_type = IOWrite;
    PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD)pClientContext, &pMyOVERLAPPED->m_ol);
    return true;
}

//这里析构才能调用 不考虑同步
void CIOCPServer::CloseCompletionPort()
{
    while (m_nWorkerCnt)
    {
        PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD)NULL, NULL);
        Sleep(100);
        InterlockedDecrement(&m_nWorkerCnt);//线程退出减一
    }
    POSITION pos = m_pClientManager->m_ClientMap.GetStartPosition();
    while (pos != NULL)
    {
        tagClientContext* pClientContext = NULL;
        SOCKET temp;
        m_pClientManager->m_ClientMap.GetNextAssoc(pos, temp, pClientContext);
        closesocket(temp);
        m_pNotifyProc(NULL, pClientContext, NC_CLIENT_DISCONNECT);
        //从队列中T出
        m_pClientManager->DeletClient(temp);
        delete pClientContext;
    }
}

bool CIOCPServer::OnClose()
{
    SetEvent(m_hKillEvent);//重置事件,监听线程退出
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

//心跳检测
void CIOCPServer::SocketCheck()
{
    if (m_pClientManager->m_ClientMap.GetCount() < 1)
    {
        Dbgprintf("Log:没有主机返回");
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
            Dbgprintf("当前检测时间%d", GetTickCount() / 1000);
            if (chazhi > timeout)
            {
                Dbgprintf("心跳超时%d", chazhi);
                closesocket(s);
                m_pClientManager->DeletClient(s);//从队列中T出
                m_pNotifyProc(NULL, pClientContext, NC_CLIENT_DISCONNECT);
                delete pClientContext;
            }
        }
    }
    LeaveCriticalSection(m_cs);
}
