#pragma once
#include "../Common/InitSock.h"
#include "../Common/Buffer.h"
#include  "../Common/DataStruct.h"
#include "../Common/zlib.h"
#ifdef _DEBUG
#pragma comment(lib,"..//Common//ZlibStatDebug//zlibstat.lib")
#else
#pragma comment(lib,"..//Common//ZlibStatRelease//zlibstat.lib")
#endif // DEBUG


#include "IMyCommand.h"

class CClientSocket
{
public:
    CClientSocket();
    ~CClientSocket();

    bool ConnectServer(CStringA csIp, int nPort);
    static unsigned __stdcall RecvProc(LPVOID lpPraram);

    bool OnRecv(char*  pBuffer, int nSize);
    bool OnSend(char*  pBuffer, int nSize);
    void OnClose();
    void SetComand(IMyCommand* pCmd);

    SOCKET      m_Socket;
private:
    CInitSock   m_InitSock;
    
    CBuffer     m_ReadBufer;        //收缓冲区
    CBuffer     m_CompressBuffer;   //解压后的缓冲区
    
    CBuffer     m_WriteBufer;       //发缓冲区
    CBuffer     m_UnCompressBuffer; //压缩缓冲区

    HANDLE      m_RecvThread;       //接收进程句柄

    IMyCommand* m_pCommand;
};

