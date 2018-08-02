#include "stdafx.h"
#include "InitSock.h"
CInitSock::CInitSock()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
        exit(0);
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        exit(0);
    }
}


CInitSock::~CInitSock()
{
    WSACleanup();
}
