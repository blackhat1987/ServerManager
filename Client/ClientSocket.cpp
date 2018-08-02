#include "stdafx.h"
#include "ClientSocket.h"


CClientSocket::CClientSocket()
    :m_pCommand(NULL)
{
    m_RecvThread = INVALID_HANDLE_VALUE;
}


CClientSocket::~CClientSocket()
{
    if (m_RecvThread != INVALID_HANDLE_VALUE)
    {
        WaitForSingleObject(m_RecvThread, INFINITE);
        CloseHandle(m_RecvThread);
        m_RecvThread = INVALID_HANDLE_VALUE;
    }
}

bool CClientSocket::ConnectServer(CStringA csIp, int nPort)
{
    u_long iMode = 1;
    m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_Socket == INVALID_SOCKET)
    {
        return false;
    }
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(csIp);
    service.sin_port = htons(nPort);
    int nRet = connect(m_Socket, (sockaddr*)&service, sizeof(service));
    if (nRet == SOCKET_ERROR)
    {
        OutputDebugStringA("Log:连接服务端失败");
        closesocket(m_Socket);
        return false;
    }
    m_RecvThread = (HANDLE)_beginthreadex(NULL, 0, RecvProc, this, 0, NULL);
    return true;
}

unsigned CClientSocket::RecvProc(LPVOID lpPraram)
{
    CClientSocket* pThis = (CClientSocket*)lpPraram;
    char Recvbuf[MAX_RECVBUF_SIZE];
    while (true)
    {
        //收消息处理
        memset(Recvbuf, 0, MAX_RECVBUF_SIZE);
        int nRet = recv(pThis->m_Socket, Recvbuf, MAX_RECVBUF_SIZE, 0);
        if (nRet <= 0)
        {
            //出错了或者服务器关闭
            OutputDebugStringA("Log:接收出错,socket线程退出");
            pThis->OnClose();
            return false;
        }
        else
        {
            pThis->OnRecv(Recvbuf, nRet);
        }
    }
    OutputDebugStringA("Log:开始接收线程退出");
    return true;
}

bool CClientSocket::OnRecv(char* pBuffer, int nSize)
{
    if (nSize == 0)
    {
        return false;
    }
    if (nSize == sizeof(tagPacketHead))
    {
        return false;
    }
    //从缓冲区把数据读出来
    m_ReadBufer.Write((PBYTE)pBuffer, nSize);
    while (m_ReadBufer.GetBufferLen() >= sizeof(tagPacketHead))
    {
        //数据验证

        //数据解压处理
        tagPacketHead pPacketHead = { 0 };
        //读取数据长度
        memcpy(&pPacketHead, m_ReadBufer.GetBuffer(0), sizeof(tagPacketHead));
        //判断数据是否完成  包总长度
        if (pPacketHead.m_dwSize > 0 && m_ReadBufer.GetBufferLen() >= pPacketHead.m_dwSize)
        {
            uLong nUnCompressLength = 0;
            //包长度-包头 = 数据长度
            int nSize = pPacketHead.m_dwSize - sizeof(tagPacketHead);
            //计算解压后的长度 数据长度
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
            m_ReadBufer.Delete(sizeof(tagPacketHead));
            //将要剩余数据复制到解压缓冲区
            m_ReadBufer.Read(pData, nSize);
            //开始解压
            if (uncompress((LPBYTE)pBuf, &nUnCompressLength, (LPBYTE)pData, nSize) != Z_OK)
            {
                delete[] pData;
                delete[] pBuf;
                OutputDebugStringA("Log:解压数据失败!");
                return false;
            }
            //消息分发
            if (m_pCommand != NULL)
            {
                //将解压后的数据复制到缓冲区
                m_CompressBuffer.ClearBuffer();
                m_CompressBuffer.Write(pBuf, nUnCompressLength);
                m_pCommand->OnRecv((char*)m_CompressBuffer.GetBuffer(0), nUnCompressLength);
            }
            delete[] pData;
            delete[] pBuf;
        }
        else
        {
            break;
        }
    }
    return false;
}

bool CClientSocket::OnSend(char* pBuffer, int nSize)
{
    m_WriteBufer.ClearBuffer();
    if (nSize >= 0)
    {
        //计算压缩后缓冲区大小
        uLong nUnSize = compressBound(nSize);
        //申请压缩后的空间
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
        m_WriteBufer.Write((PBYTE)&nPacketLen, sizeof(DWORD));
        //写入解压后的长度
        m_WriteBufer.Write((PBYTE)&nSize, sizeof(DWORD));
        //复制到发送缓冲区
        m_WriteBufer.Write(pData, nUnSize);
        //多次发送
        if (!SendData(m_Socket, (char*)m_WriteBufer.GetBuffer(), m_WriteBufer.GetBufferLen()))
        {
            return false;
        }
        delete[] pData;
        return true;
    }
    return false;
}

void CClientSocket::SetComand(IMyCommand* pCmd)
{
    m_pCommand = pCmd;
}
void CClientSocket::OnClose()
{
    if (m_Socket!=INVALID_SOCKET)
    {
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
    }
    m_pCommand->OnClose();
}