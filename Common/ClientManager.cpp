#include "stdafx.h"
#include "ClientManager.h"


CClientManager::CClientManager(CRITICAL_SECTION* cs)
    :m_cs(cs)
{
    InitializeCriticalSection(m_cs);
}


CClientManager::~CClientManager()
{

    DeleteCriticalSection(m_cs);
}


void CClientManager::AddClient(SOCKET s, CLIENT_CONTEXT* pClientContext)
{
    EnterCriticalSection(m_cs);
    m_ClientMap.SetAt(s, pClientContext);
    LeaveCriticalSection(m_cs);
}
PCLIENT_CONTEXT CClientManager::GetClient(SOCKET s)
{
    EnterCriticalSection(m_cs);
    tagClientContext* ClientContext;
    m_ClientMap.Lookup(s, ClientContext);
    LeaveCriticalSection(m_cs);
    return ClientContext;
}

void CClientManager::DeletClient(SOCKET s)
{
    EnterCriticalSection(m_cs);
    m_ClientMap.RemoveKey(s);
    LeaveCriticalSection(m_cs);
}

void CClientManager::RemoveClient()
{
    EnterCriticalSection(m_cs);
    POSITION pos = m_ClientMap.GetStartPosition();
    while (pos != NULL)
    {
        tagClientContext* ClientContext = NULL;
        SOCKET temp;
        m_ClientMap.GetNextAssoc(pos, temp, ClientContext);
        closesocket(temp);
        delete ClientContext;
    }
    LeaveCriticalSection(m_cs);
}

