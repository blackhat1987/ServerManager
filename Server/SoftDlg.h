#pragma once


// CSoftDlg �Ի���

class CSoftDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSoftDlg)

public:
	CSoftDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSoftDlg();

// �Ի�������
	enum { IDD = SoftDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedOk();

    CString m_csPort;
};
