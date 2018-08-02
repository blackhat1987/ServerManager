#pragma once


// CSoftDlg 对话框

class CSoftDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSoftDlg)

public:
	CSoftDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSoftDlg();

// 对话框数据
	enum { IDD = SoftDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedOk();

    CString m_csPort;
};
