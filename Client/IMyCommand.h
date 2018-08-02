#pragma once

class IMyCommand
{
public:
    IMyCommand();
    virtual ~IMyCommand();
    virtual void OnRecv(char*  pBuffer, int nSize) = 0;
    virtual void OnSend(char*  pBuffer, int nSize) = 0;
    virtual void OnClose() = 0;
    virtual void Execute() = 0;
private:
};

