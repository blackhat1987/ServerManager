// FileDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "FileDlg.h"
#include "afxdialogex.h"

// CFileDlg 对话框

IMPLEMENT_DYNAMIC(CFileDlg, CDialogEx)

CFileDlg::CFileDlg(CWnd* pParent, CIOCPServer* pIOCPServer , tagClientContext* pClientContext)
	: CDialogEx(CFileDlg::IDD, pParent)
{
    m_pIOCPServer = pIOCPServer;
    m_pClientContext = pClientContext;
    m_csIp = inet_ntoa(pClientContext->addr.sin_addr);
}

CFileDlg::~CFileDlg()
{
}

void CFileDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_RemoteList);
    DDX_Control(pDX, IDC_LIST2, m_LocalList);
    DDX_Control(pDX, IDC_COMBO1, m_RemoteCombox);
    DDX_Control(pDX, IDC_COMBO2, m_LocalCombox);
}


BEGIN_MESSAGE_MAP(CFileDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &CFileDlg::OnBnClickedOk)
    ON_WM_CLOSE()
    ON_CBN_SELCHANGE(IDC_COMBO2, &CFileDlg::OnCbnSelchangeCombo2)
    ON_CBN_SELCHANGE(IDC_COMBO1, &CFileDlg::OnCbnSelchangeCombo1)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST2, &CFileDlg::OnNMDblclkList2)
    ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CFileDlg::OnNMRClickList1)
    ON_NOTIFY(NM_RCLICK, IDC_LIST2, &CFileDlg::OnNMRClickList2)
    ON_COMMAND(ID_32782, &CFileDlg::OnUpload)
    ON_COMMAND(ID_32783, &CFileDlg::OnDownload)
    ON_COMMAND(MN_STOP, &CFileDlg::OnStop)
    ON_COMMAND(MN_CANCELSEND, &CFileDlg::OnCancelsend)
    ON_COMMAND(ID_32784, &CFileDlg::OnRename)
    ON_COMMAND(ID_32785, &CFileDlg::OnDelete)
    ON_COMMAND(ID_32786, &CFileDlg::OnNewfolder)

    ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CFileDlg::OnNMDblclkList1)
    
    ON_COMMAND(MN_CONTINUE, &CFileDlg::OnContinue)
    ON_COMMAND(MN_Refresh, &CFileDlg::OnRefresh)
END_MESSAGE_MAP()


// CFileDlg 消息处理程序


void CFileDlg::OnBnClickedOk()
{
    // TODO:  在此添加控件通知处理程序代码
    //CDialogEx::OnOK();
}

void CFileDlg::OnReceive()
{
    char Type;
    m_pClientContext->m_CompressBuffer.Read((PBYTE)&Type, sizeof(char));
    switch (Type)
    {
    case COMMAND_PANFU:
        InitRemoteCombox();//解析盘符
        break;
    case COMMAND_FILE_DIR:
        RemoteFileList();//解析目录
        break;
    case COMMAND_CONTINUE:
        SendFileData();//发送文件
        break;
    case COMMAND_FILE_Data://收文件
        GetFileDate();
        break;
    case COMMAND_FILE_END://发送完成
    {
        m_DownFileQueue.pop();
        AfxMessageBox(_T("传送完成"));
        break;
    }
    case COMMAND_FILE_TRANSFMODE:
        //处理传输请求
        ProcessTansfRequest();
        break;
    default:
        break;
    }
}


void CFileDlg::OnSend()
{
    //告诉客户端我准备好了
    char ch = COMMAND_FILE_BEGIN;
    m_pIOCPServer->Send(m_pClientContext, &ch, sizeof(char));
}


void CFileDlg::OnClose()
{
    // TODO:  在此添加消息处理程序代码和/或调用默认值
    m_pClientContext->m_Dialog[0] = 0;
    m_pClientContext->m_Dialog[1] = 0;
    shutdown(m_pClientContext->s, 0);
    closesocket(m_pClientContext->s);
    CDialogEx::OnClose();
}


BOOL CFileDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  在此添加额外的初始化
    CString str;
    str.Format(_T("%s---文件管理"),m_csIp);
    SetWindowText(str);
    Init();
    m_IsUpStop = false;//是否停止发送
    m_IsDownStop = false;
    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常:  OCX 属性页应返回 FALSE
}

void CFileDlg::Init()
{
    //初始化列表控件
    m_LocalList.InsertColumn(0, _T("文件名"));
    m_LocalList.ModifyStyle(0, LVS_ICON);

    m_RemoteList.InsertColumn(0, _T("文件名"),LVCFMT_LEFT, 200);
    m_RemoteList.InsertColumn(1, _T("大小"), LVCFMT_LEFT, 200);
    m_RemoteList.InsertColumn(2, _T("类型"), LVCFMT_LEFT, 200);
    //获取盘符
    TCHAR buf[128] = { 0 };
    TCHAR* pStr = NULL;
    GetLogicalDriveStrings(sizeof(buf), buf);
    pStr = buf;
    //获取系统图标
    SHFILEINFO sfi;
    HIMAGELIST hImageList;
    //获取大图标
    hImageList = (HIMAGELIST)SHGetFileInfo(_T(""), 0, &sfi, sizeof(sfi), SHGFI_LARGEICON | SHGFI_SYSICONINDEX);
    m_pImageList_Large = CImageList::FromHandle(hImageList);
    //获取小图标
    hImageList = (HIMAGELIST)SHGetFileInfo(_T(""), 0, &sfi, sizeof(sfi), LVSIL_SMALL | SHGFI_SYSICONINDEX);
    m_pImageList_Small = CImageList::FromHandle(hImageList);
    // 为列表视图设置ImageList
    m_LocalList.SetImageList(m_pImageList_Large, LVSIL_NORMAL);
    m_LocalList.SetImageList(m_pImageList_Small, LVSIL_SMALL);
    //将盘符加入本地列表
    while (*pStr)
    {
        DWORD dwFileAttribute = GetFileAttributes(pStr);
        SHFILEINFO sfi;
        SHGetFileInfo(
            pStr, dwFileAttribute,
            &sfi, sizeof(sfi),
            SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
        //如果不是本地磁盘就不添加进去
        if (GetDriveType(pStr) == DRIVE_FIXED)
        {
            TCHAR temp[4] = { 0 };
            memcpy(temp, pStr, sizeof(temp));
            m_LocalList.InsertItem(0, temp, sfi.iIcon);
            m_LocalList.SetItemData(0, 1);
            m_LocalCombox.AddString(temp);
        }
        pStr += wcslen(pStr) + 1;
    }
    m_LocalCombox.SetCurSel(0);
}

void CFileDlg::InitRemoteCombox()
{
    //解析盘符数据
    char* pStr = (char*)m_pClientContext->m_CompressBuffer.GetBuffer(0);
    int n = m_pClientContext->m_CompressBuffer.GetBufferLen() / 4;
    for (int i = 0; *pStr!='\0'; i++)
    {
        char* pszBuf = pStr;
        CString csTemp(pszBuf);
        m_RemoteCombox.AddString(csTemp);
        pStr += 4;
    }
    m_RemoteCombox.SetCurSel(0);
}

void CFileDlg::RemoteFileList()
{
    char* pStr = (char*)m_pClientContext->m_CompressBuffer.GetBuffer(0);
    int nLength = m_pClientContext->m_CompressBuffer.GetBufferLen();
    m_RemoteList.DeleteAllItems();//先清空
    for (int i = 0; i < nLength;)
    {
        char* pFileName = pStr;
        CString csFileName(pFileName);//获取文件名
        int nFileName = strlen(pFileName) + 1;//名字长度
        pStr += nFileName;      
        ULONGLONG* pFileSize = (ULONGLONG*)pStr;
        ULONGLONG nFileSize = *pFileSize;
        CString csFileSize;
        csFileSize.Format(_T("%I64u"), nFileSize);
        pStr += sizeof(nFileSize);
        CString	csType;
        if (*pStr)//根据文件类型设置图标
        {
            csType = _T("文件夹");
        }
        else
        {
            csType = _T("文件");
        }
        int nIndex = m_RemoteList.GetItemCount();
        m_RemoteList.InsertItem(nIndex, _T(""));
        m_RemoteList.SetItemText(nIndex, 0, csFileName);
        m_RemoteList.SetItemText(nIndex, 1, csFileSize);
        m_RemoteList.SetItemText(nIndex, 2, csType);
        m_RemoteList.SetItemData(nIndex, *pStr ? 1 : 0);
        pStr++;
        i += nFileName + sizeof(nFileSize) + 1;//偏移
    }
}

bool CFileDlg::OpenLocalFilePath(CString csPath)
{
    if (csPath.IsEmpty())
    {
        return false;
    }
    CFileFind Finder;
    CString csRoot;
    BOOL bWorking = Finder.FindFile(csPath + _T("*.*"));
    int nIdx = 0;
    if (bWorking)
    {
        m_LocalList.DeleteAllItems();
    }
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
        m_LocalList.InsertItem(0, Finder.GetFileName(), sfi.iIcon);
        m_LocalList.SetItemData(0, Finder.IsDirectory());
    }
    return true;
}

bool CFileDlg::OpenRemoteFilePath(CString csPath /*= NULL*/)
{
    if (csPath.IsEmpty())
    {
        return false;
    }
    char szBuf[MAX_PATH+1] = {0};
    szBuf[0] = COMMAND_FILE_DIR;
    string str = UnicodeToAscii(csPath.GetString());
    memcpy(szBuf + 1, str.data(), str.length());
    m_pIOCPServer->Send(m_pClientContext, szBuf, MAX_PATH + 1);
    return true;
}


bool CFileDlg::OnUploadFile(CString csFileName)
{
    CString csRemoteFile;
    CString csLocalFile;
    DWORD	dwSizeHigh;
    DWORD	dwSizeLow;
    //打开文件
    HANDLE hFile = CreateFile(m_csLocalCurPath + csFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    //获取文件大小
    dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
    m_nOperatingFileLength = (dwSizeHigh * (MAXDWORD + 1)) + dwSizeLow;
    CloseHandle(hFile);
    //远程路径+文件名
    csRemoteFile = m_csRemoteCurPath + csFileName;
    //本地路径+文件名
    csLocalFile = m_csLocalCurPath + csFileName;
    //计算缓冲的大小   路径文件名+大小
    int nPacket = csRemoteFile.GetLength() * 2+10;//为了国际化 这里长度要算准
    //申请缓冲区
    char* pBuf = new char[nPacket];
    if (pBuf == NULL)
    {
        return false;
    }
    memset(pBuf, 0, nPacket);
    //发送文件保存的路径和大小
    char Type = COMMAND_FILE_Size;
    //复制命令
    memcpy(pBuf, &Type, sizeof(char));
    //复制大小
    memcpy(pBuf + 1, &dwSizeHigh, sizeof(DWORD));
    memcpy(pBuf + 5, &dwSizeLow, sizeof(DWORD));
    string str = UnicodeToAscii(csRemoteFile.GetString());
    //复制文件名
    memcpy(pBuf + 9, str.data(), str.length());
    //发送文件头
    m_pIOCPServer->Send(m_pClientContext, pBuf, nPacket);
    //将本地要上传的文件绝对路径保存到队列传送的时候用
    m_UploadFileQueue.push(csLocalFile);
    delete[] pBuf;
    return true;
}


bool CFileDlg::OnDownloadFile(CString csFileName)
{
    DWORD	dwCreationDisposition;
    //远程路径+文件名
    CString csRemoteFile = m_csRemoteCurPath + csFileName;
    //本地路径+文件名
    CString csLocalFile = m_csLocalCurPath + csFileName;
    int nLength = csRemoteFile.GetLength() * 2+9;//都是国际化的锅
    char* pBuf = new char[nLength];
    if (pBuf == NULL)
    {
        return false;
    }

    //查找文件
    WIN32_FIND_DATA FindFileData = {0};
    HANDLE hFind = FindFirstFile(csLocalFile, &FindFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        //文件存在
        if (MessageBox(_T("本地已经存在该文件,YES覆盖,NO续传？"), _T("提示:"), MB_YESNO) == IDYES)
        {
            FindFileData.nFileSizeHigh = 0;
            FindFileData.nFileSizeLow = 0;
            dwCreationDisposition = CREATE_ALWAYS;
        }
        else
        {
            dwCreationDisposition = OPEN_EXISTING;
        }
       
    }
    else
    {
        dwCreationDisposition = CREATE_NEW;
    }
    FindClose(hFind);
    HANDLE	hFile = CreateFile(
        csLocalFile,
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
    //保存文件路径到 下载队列
    m_DownFileQueue.push(csLocalFile);
    string str = UnicodeToAscii(csRemoteFile.GetString());
    memset(pBuf, 0, nLength);  
    pBuf[0] = COMMAND_FILE_Down;
    //消息类型+文件偏移  +9
    //如果文件存在 获取文件大小
    memcpy(pBuf + 1, &FindFileData.nFileSizeHigh, sizeof(DWORD));
    memcpy(pBuf + 5, &FindFileData.nFileSizeLow, sizeof(DWORD));
    memcpy(pBuf + 9, str.data(), str.length());
    m_pIOCPServer->Send(m_pClientContext, pBuf, nLength);
    delete[] pBuf;
    return true;
}

bool CFileDlg::SendFileData()
{
    //取文件接受长  客户端接收多少就返回多少
    FILESIZE *pFileSize = (FILESIZE *)(m_pClientContext->m_CompressBuffer.GetBuffer());
    LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
    LONG	dwOffsetLow = pFileSize->dwSizeLow;
    //从队列中取出文件
    if (m_UploadFileQueue.size()<=0)
    {
        return false;
    }
    CString csFile = m_UploadFileQueue.front();
    //计算文件长度
    m_nCounter = MAKEINT64(pFileSize->dwSizeLow, pFileSize->dwSizeHigh);
    if (m_IsUpStop)
    {
        return true;
    }
    if (m_nCounter == m_nOperatingFileLength || pFileSize->dwSizeLow == -1 )
    {
        //从队列中清除
        m_UploadFileQueue.pop();
        AfxMessageBox(_T("文件上传完成"));
        //发送完成
        return false;
    }
    //打开文件
    HANDLE hFile = CreateFile(csFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
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
        m_pIOCPServer->Send(m_pClientContext, pFileBuf, nPacketSize);
        OutputDebugStringA("Log:数据发送成功");
    }
    delete[] pFileBuf;
    return true;
}

bool CFileDlg::GetFileDate()
{
    DWORD dwLength = m_pClientContext->m_CompressBuffer.GetBufferLen();
    //取文件传送大小
    FILESIZE	*pFileSize = NULL;
    //文件数据指针
    char*   pData = NULL;
    // 1 + 4 + 4  数据包头部大小，为固定的9
    int		nHeadLength = 9;
    DWORD	dwBytesToWrite;
    DWORD	dwBytesWrite;
    //从队列取出下载的文件路径
    if (m_DownFileQueue.size() <=0 )
    {
        return false;
    }
    CString csDownloadFilePath = m_DownFileQueue.front();
    pFileSize = (FILESIZE*)m_pClientContext->m_CompressBuffer.GetBuffer();
    pData = (char*)m_pClientContext->m_CompressBuffer.GetBuffer() + sizeof(FILESIZE);
    // 得到数据在文件中的偏移
    LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
    LONG	dwOffsetLow = pFileSize->dwSizeLow;
    //写入数据长度 要去掉8个字节
    dwBytesToWrite = dwLength - 8;
    HANDLE	hFile = CreateFile
        (
        csDownloadFilePath,
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
    if (m_IsDownStop)
    {
        return true;
    }
    char szBuf[9] = { 0 };
    szBuf[0] = COMMAND_CONTINUE;
    dwOffsetLow += dwBytesWrite;
    memcpy(szBuf + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
    memcpy(szBuf + 5, &dwOffsetLow, sizeof(dwOffsetLow));
    m_pIOCPServer->Send(m_pClientContext,szBuf, sizeof(szBuf));
    return true;
}

void CFileDlg::ProcessTansfRequest()
{
    if (MessageBox(_T("远程服务器已经存在该文件,YES覆盖,NO续传？"), _T("提示:"), MB_YESNO) == IDYES)
    {
        char ch = COMMAND_FILE_TRANSFMODE_COVER;
        m_pIOCPServer->Send(m_pClientContext, &ch, sizeof(char));
    }
    else
    {
        char ch = COMMAND_FILE_TRANSFMODE_CONTINUE;
        m_pIOCPServer->Send(m_pClientContext, &ch, sizeof(char));
    }
}

void CFileDlg::OnCbnSelchangeCombo1()
{
    // TODO:  在此添加控件通知处理程序代码
    m_RemoteCombox.GetWindowTextW(m_csRemoteCurPath);
    OpenRemoteFilePath(m_csRemoteCurPath);
}

void CFileDlg::OnCbnSelchangeCombo2()
{
    // TODO:  在此添加控件通知处理程序代码
    m_LocalCombox.GetWindowTextW(m_csLocalCurPath);
    OpenLocalFilePath(m_csLocalCurPath);
}


void CFileDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO:  在此添加控件通知处理程序代码
    int nIndex = pNMItemActivate->iItem;
    if (nIndex != -1)
    {
        if (m_RemoteList.GetItemData(nIndex) != 1)
        {
            return;
        }
        m_csRemoteFileName = m_RemoteList.GetItemText(nIndex, 0);
        if (m_csLocalCurPath.IsEmpty())
        {
            m_csLocalCurPath += m_csFileName;
        }
        else
        {
            m_csLocalCurPath += m_csFileName + _T("\\");
        }
        UpdateData(FALSE);
        OpenRemoteFilePath(m_csRemoteCurPath);
    }
    *pResult = 0;
}

void CFileDlg::OnNMDblclkList2(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO:  在此添加控件通知处理程序代码
    int nIndex = pNMItemActivate->iItem;
    if (nIndex != -1)
    {
        if (m_LocalList.GetItemData(nIndex) !=1)
        {
            return;
        }
        m_csFileName = m_LocalList.GetItemText(nIndex, 0);
        if (m_csLocalCurPath.IsEmpty())
        {
            m_csLocalCurPath += m_csFileName;
        }
        else
        {
            m_csLocalCurPath += m_csFileName + _T("\\");
        }
        UpdateData(FALSE);
        OpenLocalFilePath(m_csLocalCurPath);
    }
    *pResult = 0;
}


void CFileDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO:  在此添加控件通知处理程序代码
    int nIndex = pNMItemActivate->iItem;
    if (nIndex != -1)
    {
        CMenu menu;
        menu.LoadMenu(MN_FILE_RIGHTKEY);
        CMenu* pMenu = menu.GetSubMenu(0);
        CPoint pos;
        GetCursorPos(&pos);
        ::SetForegroundWindow(m_hWnd);
        pMenu->TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this);
    }
    *pResult = 0;
}


void CFileDlg::OnNMRClickList2(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO:  在此添加控件通知处理程序代码
    int nIndex = pNMItemActivate->iItem;
    if (nIndex != -1)
    {
        CMenu menu;
        menu.LoadMenu(MN_FILE_RIGHTKEY);
        CMenu* pMenu = menu.GetSubMenu(0);
        CPoint pos;
        GetCursorPos(&pos);
        ::SetForegroundWindow(m_hWnd);
        pMenu->TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this);
    }
    *pResult = 0;
}


void CFileDlg::OnUpload()
{
    // TODO:  在此添加命令处理程序代码
    if (GetFocus()->m_hWnd == m_LocalList.m_hWnd)
    {
        //获取上传的文件
        int nIndex = m_LocalList.GetSelectionMark();
        if (nIndex != -1 && m_LocalList.GetItemData(nIndex) != 1)
        {
            CString csFileName = m_LocalList.GetItemText(nIndex, 0);
            if (!m_UploadFileQueue.empty())
            {
                AfxMessageBox(_T("还没发送完成"));
                return;
            }
            m_IsUpStop = false;
            OnUploadFile(csFileName);
        }
    }
    else
    {
        //远程列表不响应
        return;
    }
}




void CFileDlg::OnDownload()
{
    // TODO:  在此添加命令处理程序代码
    if (GetFocus()->m_hWnd == m_LocalList.m_hWnd)
    {
        //本地不响应
        return;
    }
    else
    {
        //获取下载的文件名
        int nIndex = m_RemoteList.GetSelectionMark();
        if (nIndex != -1 && m_RemoteList.GetItemData(nIndex) != 1)
        {
            CString csFileName = m_RemoteList.GetItemText(nIndex, 0);
            if (!m_DownFileQueue.empty())
            {
                AfxMessageBox(_T("还没下载完"));
                return;
            }
            OnDownloadFile(csFileName);
        }
    }
}


void CFileDlg::OnRename()
{
    // TODO:  在此添加命令处理程序代码
    if (GetFocus()->m_hWnd == m_LocalList.m_hWnd)
    {
        return;
    }
    else
    {
        return;
    }
}


void CFileDlg::OnDelete()
{
    // TODO:  在此添加命令处理程序代码
    if (GetFocus()->m_hWnd == m_LocalList.m_hWnd)
    {
        return;
    }
    else
    {
        return;
    }
}


void CFileDlg::OnNewfolder()
{
    // TODO:  在此添加命令处理程序代码
    if (GetFocus()->m_hWnd == m_LocalList.m_hWnd)
    {
        return;
    }
    else
    {
        return;
    }
}

void CFileDlg::OnStop()
{
    // TODO:  在此添加命令处理程序代码
    if (GetFocus()->m_hWnd == m_LocalList.m_hWnd)
    {
        m_IsUpStop = true;
    }
    else
    {
        m_IsDownStop =true;
    }
}


void CFileDlg::OnCancelsend()
{
    // TODO:  在此添加命令处理程序代码
    if (GetFocus()->m_hWnd == m_LocalList.m_hWnd)
    {
        m_UploadFileQueue.pop();
    }
    else
    {
        m_DownFileQueue.pop();
    }
}


void CFileDlg::OnContinue()
{
    // TODO:  在此添加命令处理程序代码
    if (GetFocus()->m_hWnd == m_LocalList.m_hWnd)
    {
        m_IsUpStop = false;
        SendFileData();
    }
    else
    {
        m_IsDownStop = false;
        GetFileDate();
    }
}


void CFileDlg::OnRefresh()
{
    // TODO:  在此添加命令处理程序代码
    if (GetFocus()->m_hWnd == m_LocalList.m_hWnd)
    {
        OpenLocalFilePath(m_csLocalCurPath);//解析目录
    }
    else
    {
        RemoteFileList();//解析目录
    }
}
