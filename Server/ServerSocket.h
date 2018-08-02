#pragma once
// CServerSocket ÃüÁîÄ¿±ê
class CServerDlg;
class CServerSocket : public CSocket
{
private:
    CServerDlg* m_pDlg;
public:
	CServerSocket();
	virtual ~CServerSocket();
    void SetDlg(CServerDlg* pDlg);
    virtual void OnClose(int nErrorCode);
    virtual void OnAccept(int nErrorCode);
    virtual void OnReceive(int nErrorCode);
};


