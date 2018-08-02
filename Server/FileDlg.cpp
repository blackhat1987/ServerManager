// FileDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "FileDlg.h"
#include "afxdialogex.h"

// CFileDlg �Ի���

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


// CFileDlg ��Ϣ�������


void CFileDlg::OnBnClickedOk()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    //CDialogEx::OnOK();
}

void CFileDlg::OnReceive()
{
    char Type;
    m_pClientContext->m_CompressBuffer.Read((PBYTE)&Type, sizeof(char));
    switch (Type)
    {
    case COMMAND_PANFU:
        InitRemoteCombox();//�����̷�
        break;
    case COMMAND_FILE_DIR:
        RemoteFileList();//����Ŀ¼
        break;
    case COMMAND_CONTINUE:
        SendFileData();//�����ļ�
        break;
    case COMMAND_FILE_Data://���ļ�
        GetFileDate();
        break;
    case COMMAND_FILE_END://�������
    {
        m_DownFileQueue.pop();
        AfxMessageBox(_T("�������"));
        break;
    }
    case COMMAND_FILE_TRANSFMODE:
        //����������
        ProcessTansfRequest();
        break;
    default:
        break;
    }
}


void CFileDlg::OnSend()
{
    //���߿ͻ�����׼������
    char ch = COMMAND_FILE_BEGIN;
    m_pIOCPServer->Send(m_pClientContext, &ch, sizeof(char));
}


void CFileDlg::OnClose()
{
    // TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
    m_pClientContext->m_Dialog[0] = 0;
    m_pClientContext->m_Dialog[1] = 0;
    shutdown(m_pClientContext->s, 0);
    closesocket(m_pClientContext->s);
    CDialogEx::OnClose();
}


BOOL CFileDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  �ڴ���Ӷ���ĳ�ʼ��
    CString str;
    str.Format(_T("%s---�ļ�����"),m_csIp);
    SetWindowText(str);
    Init();
    m_IsUpStop = false;//�Ƿ�ֹͣ����
    m_IsDownStop = false;
    return TRUE;  // return TRUE unless you set the focus to a control
    // �쳣:  OCX ����ҳӦ���� FALSE
}

void CFileDlg::Init()
{
    //��ʼ���б�ؼ�
    m_LocalList.InsertColumn(0, _T("�ļ���"));
    m_LocalList.ModifyStyle(0, LVS_ICON);

    m_RemoteList.InsertColumn(0, _T("�ļ���"),LVCFMT_LEFT, 200);
    m_RemoteList.InsertColumn(1, _T("��С"), LVCFMT_LEFT, 200);
    m_RemoteList.InsertColumn(2, _T("����"), LVCFMT_LEFT, 200);
    //��ȡ�̷�
    TCHAR buf[128] = { 0 };
    TCHAR* pStr = NULL;
    GetLogicalDriveStrings(sizeof(buf), buf);
    pStr = buf;
    //��ȡϵͳͼ��
    SHFILEINFO sfi;
    HIMAGELIST hImageList;
    //��ȡ��ͼ��
    hImageList = (HIMAGELIST)SHGetFileInfo(_T(""), 0, &sfi, sizeof(sfi), SHGFI_LARGEICON | SHGFI_SYSICONINDEX);
    m_pImageList_Large = CImageList::FromHandle(hImageList);
    //��ȡСͼ��
    hImageList = (HIMAGELIST)SHGetFileInfo(_T(""), 0, &sfi, sizeof(sfi), LVSIL_SMALL | SHGFI_SYSICONINDEX);
    m_pImageList_Small = CImageList::FromHandle(hImageList);
    // Ϊ�б���ͼ����ImageList
    m_LocalList.SetImageList(m_pImageList_Large, LVSIL_NORMAL);
    m_LocalList.SetImageList(m_pImageList_Small, LVSIL_SMALL);
    //���̷����뱾���б�
    while (*pStr)
    {
        DWORD dwFileAttribute = GetFileAttributes(pStr);
        SHFILEINFO sfi;
        SHGetFileInfo(
            pStr, dwFileAttribute,
            &sfi, sizeof(sfi),
            SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
        //������Ǳ��ش��̾Ͳ���ӽ�ȥ
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
    //�����̷�����
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
    m_RemoteList.DeleteAllItems();//�����
    for (int i = 0; i < nLength;)
    {
        char* pFileName = pStr;
        CString csFileName(pFileName);//��ȡ�ļ���
        int nFileName = strlen(pFileName) + 1;//���ֳ���
        pStr += nFileName;      
        ULONGLONG* pFileSize = (ULONGLONG*)pStr;
        ULONGLONG nFileSize = *pFileSize;
        CString csFileSize;
        csFileSize.Format(_T("%I64u"), nFileSize);
        pStr += sizeof(nFileSize);
        CString	csType;
        if (*pStr)//�����ļ���������ͼ��
        {
            csType = _T("�ļ���");
        }
        else
        {
            csType = _T("�ļ�");
        }
        int nIndex = m_RemoteList.GetItemCount();
        m_RemoteList.InsertItem(nIndex, _T(""));
        m_RemoteList.SetItemText(nIndex, 0, csFileName);
        m_RemoteList.SetItemText(nIndex, 1, csFileSize);
        m_RemoteList.SetItemText(nIndex, 2, csType);
        m_RemoteList.SetItemData(nIndex, *pStr ? 1 : 0);
        pStr++;
        i += nFileName + sizeof(nFileSize) + 1;//ƫ��
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
    //���ļ�
    HANDLE hFile = CreateFile(m_csLocalCurPath + csFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    //��ȡ�ļ���С
    dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
    m_nOperatingFileLength = (dwSizeHigh * (MAXDWORD + 1)) + dwSizeLow;
    CloseHandle(hFile);
    //Զ��·��+�ļ���
    csRemoteFile = m_csRemoteCurPath + csFileName;
    //����·��+�ļ���
    csLocalFile = m_csLocalCurPath + csFileName;
    //���㻺��Ĵ�С   ·���ļ���+��С
    int nPacket = csRemoteFile.GetLength() * 2+10;//Ϊ�˹��ʻ� ���ﳤ��Ҫ��׼
    //���뻺����
    char* pBuf = new char[nPacket];
    if (pBuf == NULL)
    {
        return false;
    }
    memset(pBuf, 0, nPacket);
    //�����ļ������·���ʹ�С
    char Type = COMMAND_FILE_Size;
    //��������
    memcpy(pBuf, &Type, sizeof(char));
    //���ƴ�С
    memcpy(pBuf + 1, &dwSizeHigh, sizeof(DWORD));
    memcpy(pBuf + 5, &dwSizeLow, sizeof(DWORD));
    string str = UnicodeToAscii(csRemoteFile.GetString());
    //�����ļ���
    memcpy(pBuf + 9, str.data(), str.length());
    //�����ļ�ͷ
    m_pIOCPServer->Send(m_pClientContext, pBuf, nPacket);
    //������Ҫ�ϴ����ļ�����·�����浽���д��͵�ʱ����
    m_UploadFileQueue.push(csLocalFile);
    delete[] pBuf;
    return true;
}


bool CFileDlg::OnDownloadFile(CString csFileName)
{
    DWORD	dwCreationDisposition;
    //Զ��·��+�ļ���
    CString csRemoteFile = m_csRemoteCurPath + csFileName;
    //����·��+�ļ���
    CString csLocalFile = m_csLocalCurPath + csFileName;
    int nLength = csRemoteFile.GetLength() * 2+9;//���ǹ��ʻ��Ĺ�
    char* pBuf = new char[nLength];
    if (pBuf == NULL)
    {
        return false;
    }

    //�����ļ�
    WIN32_FIND_DATA FindFileData = {0};
    HANDLE hFind = FindFirstFile(csLocalFile, &FindFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        //�ļ�����
        if (MessageBox(_T("�����Ѿ����ڸ��ļ�,YES����,NO������"), _T("��ʾ:"), MB_YESNO) == IDYES)
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
    // ��Ҫ������
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    CloseHandle(hFile);
    //�����ļ�·���� ���ض���
    m_DownFileQueue.push(csLocalFile);
    string str = UnicodeToAscii(csRemoteFile.GetString());
    memset(pBuf, 0, nLength);  
    pBuf[0] = COMMAND_FILE_Down;
    //��Ϣ����+�ļ�ƫ��  +9
    //����ļ����� ��ȡ�ļ���С
    memcpy(pBuf + 1, &FindFileData.nFileSizeHigh, sizeof(DWORD));
    memcpy(pBuf + 5, &FindFileData.nFileSizeLow, sizeof(DWORD));
    memcpy(pBuf + 9, str.data(), str.length());
    m_pIOCPServer->Send(m_pClientContext, pBuf, nLength);
    delete[] pBuf;
    return true;
}

bool CFileDlg::SendFileData()
{
    //ȡ�ļ����ܳ�  �ͻ��˽��ն��پͷ��ض���
    FILESIZE *pFileSize = (FILESIZE *)(m_pClientContext->m_CompressBuffer.GetBuffer());
    LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
    LONG	dwOffsetLow = pFileSize->dwSizeLow;
    //�Ӷ�����ȡ���ļ�
    if (m_UploadFileQueue.size()<=0)
    {
        return false;
    }
    CString csFile = m_UploadFileQueue.front();
    //�����ļ�����
    m_nCounter = MAKEINT64(pFileSize->dwSizeLow, pFileSize->dwSizeHigh);
    if (m_IsUpStop)
    {
        return true;
    }
    if (m_nCounter == m_nOperatingFileLength || pFileSize->dwSizeLow == -1 )
    {
        //�Ӷ��������
        m_UploadFileQueue.pop();
        AfxMessageBox(_T("�ļ��ϴ����"));
        //�������
        return false;
    }
    //���ļ�
    HANDLE hFile = CreateFile(csFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
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
        m_pIOCPServer->Send(m_pClientContext, pFileBuf, nPacketSize);
        OutputDebugStringA("Log:���ݷ��ͳɹ�");
    }
    delete[] pFileBuf;
    return true;
}

bool CFileDlg::GetFileDate()
{
    DWORD dwLength = m_pClientContext->m_CompressBuffer.GetBufferLen();
    //ȡ�ļ����ʹ�С
    FILESIZE	*pFileSize = NULL;
    //�ļ�����ָ��
    char*   pData = NULL;
    // 1 + 4 + 4  ���ݰ�ͷ����С��Ϊ�̶���9
    int		nHeadLength = 9;
    DWORD	dwBytesToWrite;
    DWORD	dwBytesWrite;
    //�Ӷ���ȡ�����ص��ļ�·��
    if (m_DownFileQueue.size() <=0 )
    {
        return false;
    }
    CString csDownloadFilePath = m_DownFileQueue.front();
    pFileSize = (FILESIZE*)m_pClientContext->m_CompressBuffer.GetBuffer();
    pData = (char*)m_pClientContext->m_CompressBuffer.GetBuffer() + sizeof(FILESIZE);
    // �õ��������ļ��е�ƫ��
    LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
    LONG	dwOffsetLow = pFileSize->dwSizeLow;
    //д�����ݳ��� Ҫȥ��8���ֽ�
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
    if (MessageBox(_T("Զ�̷������Ѿ����ڸ��ļ�,YES����,NO������"), _T("��ʾ:"), MB_YESNO) == IDYES)
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
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    m_RemoteCombox.GetWindowTextW(m_csRemoteCurPath);
    OpenRemoteFilePath(m_csRemoteCurPath);
}

void CFileDlg::OnCbnSelchangeCombo2()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    m_LocalCombox.GetWindowTextW(m_csLocalCurPath);
    OpenLocalFilePath(m_csLocalCurPath);
}


void CFileDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
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
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
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
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
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
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
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
    // TODO:  �ڴ���������������
    if (GetFocus()->m_hWnd == m_LocalList.m_hWnd)
    {
        //��ȡ�ϴ����ļ�
        int nIndex = m_LocalList.GetSelectionMark();
        if (nIndex != -1 && m_LocalList.GetItemData(nIndex) != 1)
        {
            CString csFileName = m_LocalList.GetItemText(nIndex, 0);
            if (!m_UploadFileQueue.empty())
            {
                AfxMessageBox(_T("��û�������"));
                return;
            }
            m_IsUpStop = false;
            OnUploadFile(csFileName);
        }
    }
    else
    {
        //Զ���б���Ӧ
        return;
    }
}




void CFileDlg::OnDownload()
{
    // TODO:  �ڴ���������������
    if (GetFocus()->m_hWnd == m_LocalList.m_hWnd)
    {
        //���ز���Ӧ
        return;
    }
    else
    {
        //��ȡ���ص��ļ���
        int nIndex = m_RemoteList.GetSelectionMark();
        if (nIndex != -1 && m_RemoteList.GetItemData(nIndex) != 1)
        {
            CString csFileName = m_RemoteList.GetItemText(nIndex, 0);
            if (!m_DownFileQueue.empty())
            {
                AfxMessageBox(_T("��û������"));
                return;
            }
            OnDownloadFile(csFileName);
        }
    }
}


void CFileDlg::OnRename()
{
    // TODO:  �ڴ���������������
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
    // TODO:  �ڴ���������������
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
    // TODO:  �ڴ���������������
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
    // TODO:  �ڴ���������������
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
    // TODO:  �ڴ���������������
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
    // TODO:  �ڴ���������������
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
    // TODO:  �ڴ���������������
    if (GetFocus()->m_hWnd == m_LocalList.m_hWnd)
    {
        OpenLocalFilePath(m_csLocalCurPath);//����Ŀ¼
    }
    else
    {
        RemoteFileList();//����Ŀ¼
    }
}
