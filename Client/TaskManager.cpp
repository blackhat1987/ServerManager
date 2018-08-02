#include "stdafx.h"
#include "TaskManager.h"


CTaskManager::CTaskManager()
{
}


CTaskManager::~CTaskManager()
{
}

bool CTaskManager::Add(IMyCommand* pCmd)
{
    m_CriticalSection.Lock();
    m_qCommand.push(pCmd);
    m_CriticalSection.Unlock();
    return false;
}

bool CTaskManager::Get(IMyCommand*& pCmd)
{
    m_CriticalSection.Lock();
    pCmd = m_qCommand.front();
    m_qCommand.pop();
    m_CriticalSection.Unlock();
    return true;
}
