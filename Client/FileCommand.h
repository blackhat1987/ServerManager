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

    void    GetDisk();//��ȡ�̷�
    bool    OpenLocalFilePath(char* pBuffer); //��Ŀ¼
    void    CreateLocalRecvFile(char* pBuffer);// �������������ܵ����ļ�

    bool    GetFileData();//��ȡ�ļ�����
    bool    WriteLocalFile(char* pBuffer,int nSize);//д�ļ�������

    bool    UploadFile(char* pBuffer);//��Ӧ��������������
    bool    SendFileData(char* pBuffer);             //��Ӧ�������ļ�������������
    
    bool    NewCreateFlie();            //����
    
    __int64 m_nOperatingFileLength; // �ļ��ܴ�С
    __int64	m_nCounter;// ������

private:
    CClientSocket m_ClientSocket;
    char*       m_szIp;
    int         m_nPort;
    HANDLE      m_hEvent;
    DWORD       m_dwLastTime;
   
    CString     m_csDownloadFilePath; //������д�뵽���ص��ļ�·�� 
    CString     m_csUploadFilePath; //������Ҫ���ص��ļ�·��

};

