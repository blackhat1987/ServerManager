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
        OutputDebugStringA("Log:���ӷ����ʧ��");
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
        //����Ϣ����
        memset(Recvbuf, 0, MAX_RECVBUF_SIZE);
        int nRet = recv(pThis->m_Socket, Recvbuf, MAX_RECVBUF_SIZE, 0);
        if (nRet <= 0)
        {
            //�����˻��߷������ر�
            OutputDebugStringA("Log:���ճ���,socket�߳��˳�");
            pThis->OnClose();
            return false;
        }
        else
        {
            pThis->OnRecv(Recvbuf, nRet);
        }
    }
    OutputDebugStringA("Log:��ʼ�����߳��˳�");
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
    //�ӻ����������ݶ�����
    m_ReadBufer.Write((PBYTE)pBuffer, nSize);
    while (m_ReadBufer.GetBufferLen() >= sizeof(tagPacketHead))
    {
        //������֤

        //���ݽ�ѹ����
        tagPacketHead pPacketHead = { 0 };
        //��ȡ���ݳ���
        memcpy(&pPacketHead, m_ReadBufer.GetBuffer(0), sizeof(tagPacketHead));
        //�ж������Ƿ����  ���ܳ���
        if (pPacketHead.m_dwSize > 0 && m_ReadBufer.GetBufferLen() >= pPacketHead.m_dwSize)
        {
            uLong nUnCompressLength = 0;
            //������-��ͷ = ���ݳ���
            int nSize = pPacketHead.m_dwSize - sizeof(tagPacketHead);
            //�����ѹ��ĳ��� ���ݳ���
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
            m_ReadBufer.Delete(sizeof(tagPacketHead));
            //��Ҫʣ�����ݸ��Ƶ���ѹ������
            m_ReadBufer.Read(pData, nSize);
            //��ʼ��ѹ
            if (uncompress((LPBYTE)pBuf, &nUnCompressLength, (LPBYTE)pData, nSize) != Z_OK)
            {
                delete[] pData;
                delete[] pBuf;
                OutputDebugStringA("Log:��ѹ����ʧ��!");
                return false;
            }
            //��Ϣ�ַ�
            if (m_pCommand != NULL)
            {
                //����ѹ������ݸ��Ƶ�������
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
        //����ѹ���󻺳�����С
        uLong nUnSize = compressBound(nSize);
        //����ѹ����Ŀռ�
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
        m_WriteBufer.Write((PBYTE)&nPacketLen, sizeof(DWORD));
        //д���ѹ��ĳ���
        m_WriteBufer.Write((PBYTE)&nSize, sizeof(DWORD));
        //���Ƶ����ͻ�����
        m_WriteBufer.Write(pData, nUnSize);
        //��η���
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