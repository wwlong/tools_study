#ifndef CLIENT_H_
#define CLIENT_H_

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include <string>
#include <list>
#include <vector>
#include <memory>

#define FTP_FAIL -1
#define FTP_SUCC 0

#define FTP_TRUE 1
#define FTP_FALSE 0

#define INVALID_SOCKET				-1
#define FTP_API						int
#define MAX_PATH					512
#define trace					        
#define warn						printf

#define FTP_DEFAULT_PORT			21						//FTP默认端口号
#define FTP_DEFAULT_BUFFER			1024*4							//FTP下载缓冲默认大小
	
#define FTP_COMMAND_BASE			1000
#define FTP_COMMAND_END				FTP_COMMAND_BASE + 30
#define FTP_COMMAND_USERNAME		FTP_COMMAND_BASE + 1			//用户名
#define FTP_COMMAND_PASSWORD		FTP_COMMAND_BASE + 2			//密码
#define FTP_COMMAND_QUIT			FTP_COMMAND_BASE + 3			//退出
#define FTP_COMMAND_CURRENT_PATH	FTP_COMMAND_BASE + 4			// 获取文件路径
#define FTP_COMMAND_TYPE_MODE		FTP_COMMAND_BASE + 5			// 改变传输模式
#define FTP_COMMAND_PSAV_MODE		FTP_COMMAND_BASE + 6			// 被动端口模式
#define FTP_COMMAND_DIR				FTP_COMMAND_BASE + 7			// 获取文件列表
#define FTP_COMMAND_CHANGE_DIRECTORY FTP_COMMAND_BASE + 8			// 改变路径
#define FTP_COMMAND_DELETE_FILE		FTP_COMMAND_BASE + 9			// 删除文件
#define FTP_COMMAND_DELETE_DIRECTORY FTP_COMMAND_BASE + 10			// 删除目录/文件夹
#define FTP_COMMAND_CREATE_DIRECTORY FTP_COMMAND_BASE + 11			// 创建目录/文件夹
#define FTP_COMMAND_RENAME_BEGIN    FTP_COMMAND_BASE  +12			// 开始重命名
#define FTP_COMMAND_RENAME_END      FTP_COMMAND_BASE + 13			// 重命名结束
#define FTP_COMMAND_FILE_SIZE		FTP_COMMAND_BASE + 14			// 获取文件大小
#define FTP_COMMAND_DOWNLOAD_POS	FTP_COMMAND_BASE + 15			// 下载文件从指定位置开始
#define FTP_COMMAND_DOWNLOAD_FILE	FTP_COMMAND_BASE + 16			// 下载文件
#define FTP_COMMAND_UPLOAD_FILE		FTP_COMMAND_BASE + 17			// 上传文件
#define FTP_COMMAND_APPEND_FILE		FTP_COMMAND_BASE + 18			// 追加上载文件	
#define FTP_COMMAND_PORT_MODE		FTP_COMMAND_BASE + 19			// 主动模式	
#define FTP_COMMAND_NOOP				FTP_COMMAND_BASE + 20
#define FTP_COMMAND_NLIST				FTP_COMMAND_BASE + 21


/*		  登陆步骤
		login2Server
			|
		inputUserName
			|
		inputPassWord
			|
		  具体操作
			|
		  quit
*/

struct FTPParams {
        std::string ip;
        unsigned int port;
        std::string username;
        std::string password;
        std::string path;
};

class CFTPManager 
{
public :
	enum type {
		binary = 0x31,
		ascii,
	};

	CFTPManager(const FTPParams &tcp_params);
	virtual ~CFTPManager(void);

	// !设置传输格式 2进制  还是ascii方式传输
	FTP_API setTransferMode(type mode);

	//list filename
	const std::string Nlist(const std::string &path, int cmd_type);

	// !命令： CD
	FTP_API CD(const std::string &path);

	// ! 创建目录/文件夹
	FTP_API CreateDirectory(const std::string &strRemoteDir);

	// !重命名
	FTP_API Rename(const std::string &strRemoteFile, const std::string &strNewFile);

	// !关闭连接
	void Close(int &sock);

	FTP_API Push(std::string FilePrefix, char *data, int size);

	FTP_API DelFile(const std::string &strRemoteFile);

	FTP_API DelTemps(std::string path);

	long GetFileLen(const std::string &strRemoteFile);
	// !合成发送到服务器的命令
	const std::string composeCommand(const unsigned int command, const std::string &strParam);

	// !发送命令
	FTP_API Send(int fd, const std::string &cmd);

	// ! 返回服务器信息
	const std::string serverResponse(int sockfd);

	// 解析返回ftp命令的值
	FTP_API parseResponse(const std::string &str);
	
	// !服务器的IP
	std::string m_strServerIP;

	// !服务器Port
	unsigned int m_nServerPort;

	// !当前用户名
	std::string m_strUserName;

	// !当前用户密码
	std::string m_strPassWord;

private:

	// ! 连接服务器
	FTP_API connect2Server();

	// !输入用户名
	FTP_API inputUserName();

	// !输入密码
	FTP_API inputPassWord();

	// !退出FTP
	FTP_API quitServer(void);

	// !设置为被动模式
	const std::string Pasv();

	// !设置为主动模式(默认)
	const std::string Port(std::string local_ip, int data_port);

	// ! 建立连接
	FTP_API Connect(int socketfd, const std::string &serverIP, unsigned int nPort);

	// !获取服务器数据
	FTP_API getData(int fd, char *strBuf, unsigned long length);

	// !发送命令
	FTP_API Send(int fd, const char *cmd, const size_t len);

	// !建立数据连接
	FTP_API createDataLink(int data_fd);

	FTP_API createDataLink(int data_fd, int accp_fd);

	//！控制连接套接字
	int m_cmdSocket;
	
	// !服务器回应信息缓存
	std::string m_strResponse;

	// !保存命令参数
	std::string m_commandStr;

	// ！当前使用的命令参数
	unsigned int m_nCurrentCommand;

	// !是否登陆标志。
	bool m_bLogin;

	std::string m_strPath;

	int cycle_cout;
};
#endif
