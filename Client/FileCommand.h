#pragma once
#include "IMyCommand.h"
#include "ClientSocket.h"

#define FILE_SEND_SIZE 1024*10

class CFileCommand :
    public IMyCommand
{
public:
    CFileCommand(char* szIP, int nPort);
    virtual ~CFileCommand();
    virtual void OnRecv(char* pBuffer, int nSize);
    virtual void OnSend(char* pBuffer, int nSize);
    virtual void Execute();
    virtual void OnClose();

    void    GetDisk();//获取盘符
    bool    OpenLocalFilePath(char* pBuffer); //打开目录
    void    CreateLocalRecvFile(char* pBuffer);// 创建服务器接受到的文件

    bool    GetFileData();//获取文件数据
    bool    WriteLocalFile(char* pBuffer,int nSize);//写文件到本地

    bool    UploadFile(char* pBuffer);//响应服务器下载请求
    bool    SendFileData(char* pBuffer);             //响应服务器文件下载数据请求
    
    bool    NewCreateFlie();            //覆盖
    
    __int64 m_nOperatingFileLength; // 文件总大小
    __int64	m_nCounter;// 计数器

private:
    CClientSocket m_ClientSocket;
    char*       m_szIp;
    int         m_nPort;
    HANDLE      m_hEvent;
    DWORD       m_dwLastTime;
   
    CString     m_csDownloadFilePath; //服务器写入到本地的文件路径 
    CString     m_csUploadFilePath; //服务器要下载的文件路径

};

