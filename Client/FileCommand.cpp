#include "stdafx.h"
#include "FileCommand.h"
#include "../Common/Buffer.h"
#include "../Common/common.h"


CFileCommand::CFileCommand(char* szIP, int nPort)
{
    m_szIp = szIP;
    m_nPort = nPort;
    m_ClientSocket.SetComand(this);
    m_hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
}

CFileCommand::~CFileCommand()
{
}

void CFileCommand::OnRecv(char* pBuffer, int nSize)
{
    //�������ݵ�ʱ����ȥ������
    switch (pBuffer[0])
    {
    case COMMAND_FILE_BEGIN:
        //�����̷�
        GetDisk();
        break;
    case  COMMAND_FILE_DIR:
        //ö���ļ���
        OpenLocalFilePath(pBuffer+1);
        break;
    case COMMAND_FILE_Size:
        //���ļ�ͷ
        CreateLocalRecvFile(pBuffer + 1);
        break;
    case COMMAND_FILE_Data:
        //д�ļ�
        WriteLocalFile(pBuffer + 1, nSize-1);
        break;
    case COMMAND_FILE_Down:
        //��Ӧ����������
        UploadFile(pBuffer + 1);
        break;
    case COMMAND_CONTINUE:
        //��Ӧ������������������
        SendFileData(pBuffer + 1);
        break;
    case COMMAND_FILE_TRANSFMODE_COVER:
        NewCreateFlie();//����
        break;
    case COMMAND_FILE_TRANSFMODE_CONTINUE:
        GetFileData();//����
        break;
    case COMMAND_HEARTBEAT:       //����
        m_dwLastTime = GetTickCount() / 1000;
        break;
    default:
        break;
    }
}

void CFileCommand::OnSend(char* pBuffer, int nSize)
{
    m_ClientSocket.OnSend(pBuffer, nSize);
}

void CFileCommand::Execute()
{
    OutputDebugStringA("Log:FileCmd��ʼִ��");
    bool bRet = m_ClientSocket.ConnectServer(m_szIp, m_nPort);
    if (!bRet)
    {
        return;
    }
    m_dwLastTime = GetTickCount() / 1000;
    char bCommand = COMMAND_OPEN_FILEDLG;
    OutputDebugStringA("Log:���ʹ�FILE������Ϣ");
    OnSend(&bCommand, sizeof(char));

    //�ȴ��¼�
    WaitForSingleObject(m_hEvent, INFINITE);
    OutputDebugStringA("Log:CFileCommand::Execute()ִ�����");
}

void CFileCommand::OnClose()
{
    SetEvent(m_hEvent);
}
//��ȡ�����̷�
void CFileCommand::GetDisk()
{
    //��ȡ�̷�
    char buf[MAXBYTE] = { 0 };
    buf[0] = COMMAND_PANFU;
    GetLogicalDriveStringsA(MAXBYTE - 1, buf+1);
    OnSend(buf, MAXBYTE);
    OutputDebugStringA("Log:�����̷����");
}

bool CFileCommand::OpenLocalFilePath(char* pBuffer)
{
    CString csPath(pBuffer);
    CBuffer DataBuf;
    char Type = COMMAND_FILE_DIR;
    DataBuf.Write((PBYTE)&Type, sizeof(char));
    if (csPath.IsEmpty())
    {
        return false;
    }
    CFileFind Finder;
    CString csRoot;
    BOOL bWorking = Finder.FindFile(csPath + _T("*.*"));
    while (bWorking)
    {
        bWorking = Finder.FindNextFile();
        CString str = Finder.GetFilePath();
        csRoot = Finder.GetRoot();
        DWORD dwFileAttribute = GetFileAttributes(str);
        SHFILEINFO sfi;
        SHGetFileInfo(str,
            dwFileAttribute,
            &sfi, sizeof(sfi),
            SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
        CString csFileName = Finder.GetFileName();//�ļ���
        char ch = Finder.IsDirectory();//�Ƿ���Ŀ¼
        ULONGLONG nFileSize = Finder.GetLength();
        string strFileName = UnicodeToAscii(csFileName.GetString());
        DataBuf.Write((PBYTE)strFileName.data(), strFileName.length()+1);//�ļ���
        DataBuf.Write((PBYTE)&nFileSize, sizeof(nFileSize));//�ļ���С
        DataBuf.Write((PBYTE)&ch, sizeof(char));//�Ƿ���Ŀ¼ 
    }
    OnSend((char*)DataBuf.GetBuffer(), DataBuf.GetBufferLen());
    return true;
}

void CFileCommand::CreateLocalRecvFile(char* pBuffer)
{
    //��ȡ�ļ���С
    FILESIZE	*pFileSize = (FILESIZE *)pBuffer;
    //�����ļ�·��
    char szFilePath[MAX_PATH] = {0};
    //+8ȥ���ļ�����
    strcpy_s(szFilePath,pBuffer + 8);
    
    CString    csTemp(szFilePath);
    m_csDownloadFilePath = csTemp;
    //�����ļ�·�������ļ���
    MakeSureDirectoryPathExists(m_csDownloadFilePath);
    //�����ļ�
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile(m_csDownloadFilePath, &FindFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        //�ļ����� ���ǻ�������  ������ģʽ
        char Type = COMMAND_FILE_TRANSFMODE;
        OnSend(&Type, sizeof(char));
    }
    else
    {
        //�ļ������������������������
        GetFileData();
    }
    FindClose(hFind);
}

bool CFileCommand::GetFileData()
{
    DWORD	dwCreationDisposition;

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile(m_csDownloadFilePath, &FindFileData);
    char szBuf[9] = { 0 };
    char Type = COMMAND_CONTINUE;
    memcpy(szBuf, &Type, sizeof(char));
    if (hFind != INVALID_HANDLE_VALUE)
    {
        //����ļ����� ��ȡ�ļ���С ���������ļ���д��ָ��
        memcpy(szBuf + 1, &FindFileData.nFileSizeHigh, 4);
        memcpy(szBuf + 5, &FindFileData.nFileSizeLow, 4);
        //���Ѵ���
        dwCreationDisposition = OPEN_EXISTING;
    }
    else
    {
        //�������½�
        dwCreationDisposition = CREATE_NEW;
    }
    FindClose(hFind);
    //�����ļ���־���ļ�
    HANDLE	hFile =CreateFile(
        m_csDownloadFilePath,
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        dwCreationDisposition,
        FILE_ATTRIBUTE_NORMAL,
        0
        );
    // ��Ҫ������
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    CloseHandle(hFile);
    //��������
    OnSend(szBuf, sizeof(szBuf));
    return true;
}

bool CFileCommand::WriteLocalFile(char* pBuffer, int nSize)
{
    Dbgprintf("Log:�յ��ļ�����%d", nSize);
    //ȡ�ļ����ʹ�С
    FILESIZE	*pFileSize = NULL;
    // 1 + 4 + 4  ���ݰ�ͷ����С��Ϊ�̶���9
    int		nHeadLength = 9; 
    DWORD	dwBytesToWrite;
    DWORD	dwBytesWrite;
    //�ļ�����ָ��
    char*   pData = NULL;
    pFileSize = (FILESIZE*)pBuffer;
    pData = pBuffer + sizeof(FILESIZE);
    // �õ��������ļ��е�ƫ��
    LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
    LONG	dwOffsetLow = pFileSize->dwSizeLow;
    //д�����ݳ��� Ҫȥ��8���ֽ�
    dwBytesToWrite = nSize - 8;
    HANDLE	hFile =CreateFile
        (
        m_csDownloadFilePath,
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0
        );
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    SetFilePointer(hFile, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);
    int nRet = 0;
    // д���ļ�
    nRet = WriteFile
        (
        hFile,
        pData,
        dwBytesToWrite,
        &dwBytesWrite,
        NULL
        );
    CloseHandle(hFile);
    //������һ�η���
    char szBuf[9] = { 0 };
    szBuf[0] = COMMAND_CONTINUE;
    dwOffsetLow += dwBytesWrite;
    memcpy(szBuf + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
    memcpy(szBuf + 5, &dwOffsetLow, sizeof(dwOffsetLow));
    OnSend(szBuf, sizeof(szBuf));
    return true;
}

bool CFileCommand::UploadFile(char* pBuffer)
{
    DWORD	dwSizeHigh;
    DWORD	dwSizeLow;
    //�����ļ�·��
    char szFilePath[MAX_PATH] = { 0 };
    strcpy_s(szFilePath,pBuffer+8);
    m_csUploadFilePath = szFilePath;
    //���ļ�
    HANDLE hFile = CreateFile(m_csUploadFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    //��ȡ�ļ���С
    dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
    m_nOperatingFileLength = (dwSizeHigh * (MAXDWORD + 1)) + dwSizeLow;
    CloseHandle(hFile);
    //����������ʼ��   �����ܱ�֤�ļ�����
    SendFileData(pBuffer);
    return true;
}

bool CFileCommand::SendFileData(char* pBuffer)
{
    //���ļ���С
    FILESIZE* pFileSize = (FILESIZE*)pBuffer;
    LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
    LONG	dwOffsetLow = pFileSize->dwSizeLow;
    m_nCounter = MAKEINT64(pFileSize->dwSizeLow, pFileSize->dwSizeHigh);
    if (m_nCounter == m_nOperatingFileLength || pFileSize->dwSizeLow == -1)
    {
        //֪ͨ�������������
        char Type = COMMAND_FILE_END;
        OnSend(&Type, sizeof(Type));
    }
    //���ļ�
    HANDLE hFile = CreateFile(m_csUploadFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    //ÿ�����������ļ�ָ��
    SetFilePointer(hFile, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);
    int nHeadLength = 9; // 1 + 4 + 4  ���ݰ�ͷ����С��Ϊ�̶���9
    //����ÿ�ζ�����
    DWORD	nNumberOfBytesToRead = FILE_SEND_SIZE - nHeadLength;
    //���淵����
    DWORD	nNumberOfBytesRead = 0;
    //���仺����
    char* pFileBuf = new char[FILE_SEND_SIZE];
    if (pFileBuf == NULL)
    {
        return false;
    }
    memset(pFileBuf, 0, nNumberOfBytesToRead);
    //��д������
    char Type = COMMAND_FILE_Data;
    memcpy(pFileBuf, &Type, sizeof(char));
    memcpy(pFileBuf + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));  //�ļ�ָ��
    memcpy(pFileBuf + 5, &dwOffsetLow, sizeof(dwOffsetLow));    //�ļ�ָ��
    ReadFile(hFile, pFileBuf + nHeadLength, nNumberOfBytesToRead, &nNumberOfBytesRead, NULL);
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
    //�������ݷ���
    if (nNumberOfBytesRead > 0)
    {
        int	nPacketSize = nNumberOfBytesRead + nHeadLength;
        OnSend(pFileBuf, nPacketSize);
        OutputDebugStringA("Log:���ݷ��ͳɹ�");
    }
    delete[] pFileBuf;
    return true;
}

bool CFileCommand::NewCreateFlie()
{
    //����һ���µ��ļ�����
    HANDLE	hFile = CreateFile(
        m_csDownloadFilePath,
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        0
        );
    // ��Ҫ������
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    CloseHandle(hFile);
    GetFileData();
    return true;
}

