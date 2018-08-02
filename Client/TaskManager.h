#pragma once
#include "IMyCommand.h"
#include <queue>
using namespace std;
class CTaskManager
{
public:
    CTaskManager();
    ~CTaskManager();
    bool Add(IMyCommand* pCmd);
    bool Get(IMyCommand*& pCmd);
private:
    queue<IMyCommand*> m_qCommand;
    CCriticalSection m_CriticalSection;
}; 

