#pragma once

#include "IOCPServer.h"
#include "../Common/common.h"
#include "../Common/DataStruct.h"
#include "afxcmn.h"
#include "afxwin.h"
#include <queue>
// CFileDlg 对话框
#define FILE_SEND_SIZE 1024*10
class CFileDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileDlg)

public:
    CFileDlg(CWnd* pParent = NULL,CIOCPServer* pIOCPServer = NULL, tagClientContext* pClientContext = NULL);   // 标准构造函数
	virtual ~CFileDlg();

// 对话框数据
	enum { IDD = FileDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnClose();
    virtual BOOL OnInitDialog();

    CIOCPServer*        m_pIOCPServer;
    tagClientContext*   m_pClientContext;

    void OnReceive();
    void OnSend();
    void Init();

    void InitRemoteCombox(); //初始化远程盘符
    void RemoteFileList();//解析获取到的目录数据

    bool OpenLocalFilePath(CString csPath = NULL);
    bool OpenRemoteFilePath(CString csPath = NULL);

    bool OnUploadFile(CString csFileName);//上传
    bool OnDownloadFile(CString csFileName);//下载

    bool SendFileData();//发送数据
    bool GetFileDate();//处理收到的数据

    void ProcessTansfRequest();//处理传输请求

    CImageList* m_pImageList_Large;
    CImageList* m_pImageList_Small;

    CListCtrl m_RemoteList;
    CListCtrl m_LocalList;
    CComboBox m_RemoteCombox;
    CComboBox m_LocalCombox;

    CString  m_csLocalCurPath;
    CString  m_csRemoteCurPath;
    CString  m_csFileName;
    CString  m_csRemoteFileName;

    __int64 m_nOperatingFileLength; // 文件总大小
    __int64	m_nCounter;// 计数器

    queue<CString> m_UploadFileQueue;//上传队列
    queue<CString> m_DownFileQueue;//下载队列

    bool    m_IsUpStop;
    bool    m_IsDownStop;

    CString m_csIp;

    afx_msg void OnCbnSelchangeCombo1();
    afx_msg void OnCbnSelchangeCombo2();
    afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMDblclkList2(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMRClickList2(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnUpload();
    afx_msg void OnDownload();
    afx_msg void OnRename();
    afx_msg void OnDelete();
    afx_msg void OnNewfolder();
    afx_msg void OnStop();
    afx_msg void OnCancelsend();
    afx_msg void OnContinue();
    afx_msg void OnRefresh();
};
