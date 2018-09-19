#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <sys/socket.h>  
#include <netinet/tcp.h> 
#include <signal.h>
#include <sys/types.h>          /* See NOTES */
#include <net/if.h>

#include "FtpClientLib.h"

using namespace std;


static int SplitString( std::string strSrc, std::list<std::string> &strArray , std::string strFlag){
	int pos = 1; 

	while((pos = (int)strSrc.find_first_of(strFlag.c_str())) > 0) {
		strArray.push_back(strSrc.substr(0 , pos));
		strSrc = strSrc.substr(pos + 1, strSrc.length() - pos - 1); 
	}

	if(!strSrc.empty()){
		strArray.push_back(strSrc);
	}

	return FTP_SUCC; 
}

std::string CompositString(std::string ip, int port){
	std::string dst;
	int pos = 1;
	
	while((pos = (int)ip.find_first_of(".")) > 0) {
		dst += ip.substr(0 , pos);
		dst += ",";
		ip = ip.substr(pos + 1, ip.length() - pos - 1); 
	}

	dst += ip;
	dst += ",";
	dst += std::to_string(port/256);
	dst += ",";
	dst += std::to_string(port%256);

	return dst;
}

int ParseString(std::list<std::string> strArray, unsigned long & nPort ,std::string & strServerIp){
	if (strArray.size() < 6 )
		return FTP_FAIL ;

	std::list<std::string>::iterator citor;
	citor = strArray.begin();
	strServerIp = *citor;
	strServerIp += ".";
	citor ++;
	strServerIp += *citor;
	strServerIp += ".";
	citor ++ ;
	strServerIp += *citor;
	strServerIp += ".";
	citor ++ ;
	strServerIp += *citor;
	citor = strArray.end();
	citor--;
	nPort = atol( (*citor).c_str());
	citor--;
	nPort += atol( (*(citor)).c_str()) * 256 ;

	return FTP_SUCC;
}

std::string GetLocalIp(){

	int sock_get_ip;
	char ipaddr[50];

	struct   sockaddr_in *sin;
	struct   ifreq ifr_ip;

	if ((sock_get_ip=socket(AF_INET, SOCK_STREAM, 0)) == -1){
		warn("socket create failse...GetLocalIp!/n");
		return "";
	}

	memset(&ifr_ip, 0, sizeof(ifr_ip));
	strncpy(ifr_ip.ifr_name, "eth0", sizeof(ifr_ip.ifr_name) - 1);
	if(ioctl(sock_get_ip, SIOCGIFADDR, &ifr_ip) < 0 )
	{
		return "";
	}
	sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;
	strcpy(ipaddr,inet_ntoa(sin->sin_addr));
	close(sock_get_ip);

	return ipaddr;
}

CFTPManager::CFTPManager(const FTPParams &ftp_params){
	m_cmdSocket = 0;
	m_strResponse = "";
	m_commandStr = "";
	m_nCurrentCommand = 0;
	m_bLogin = false;
	m_strPath = "";
	cycle_cout = 0;

	signal(SIGPIPE, SIG_IGN);
	
	struct hostent *addr_info = NULL;
	addr_info = gethostbyname(ftp_params.ip.c_str());
	if(addr_info == NULL){
		warn("!!! can not fetch serverip from host!\n");
		m_strServerIP = "";
	}else{
		char str[32];
		inet_ntop(addr_info->h_addrtype, addr_info->h_addr, str, sizeof(str));
		m_strServerIP = str;
		printf("Got ServerIP %s\n", str);
	}

	m_nServerPort = ftp_params.port;
	m_strUserName = ftp_params.username;
	m_strPassWord = ftp_params.password;
	m_strPath = ftp_params.path;	
}

CFTPManager::~CFTPManager(void){
	if(m_cmdSocket > 0){
		Close(m_cmdSocket);
		m_bLogin = false;
	}
}

FTP_API CFTPManager::connect2Server(){

	int m_retry_times = 0;
	int ret = FTP_SUCC;
	
M_RETRY:
	m_cmdSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(m_cmdSocket <= 0){
		m_retry_times++;
		if(m_retry_times > 3){
			warn("!!! ftp cmd socket create retry limited!\n");
			return FTP_FAIL;
		}
		
		goto M_RETRY;
	}

	//set port reused
	int addruse = 1;
	setsockopt(m_cmdSocket, SOL_SOCKET, SO_REUSEADDR, &addruse, sizeof(addruse));

	//keep alive sets
	int keepAlive = 1; //open keepalive
	int keepIdle = 5; // timeout to start check.
	int keepInterval = 1; // time between to check
	int keepCount = 3; // check total times.

	setsockopt(m_cmdSocket, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
	setsockopt(m_cmdSocket, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
	setsockopt(m_cmdSocket, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
	setsockopt(m_cmdSocket, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));

	if(m_strServerIP.empty()){
		warn("!!! do not get valid ip!\n");
		close(m_cmdSocket);
		m_cmdSocket = INVALID_SOCKET;
		return FTP_FAIL;
	}

	if(m_nServerPort <= 0){
		m_nServerPort = FTP_DEFAULT_PORT;
		warn("!!! connect to server:%s,use default port:%d\n",m_strServerIP.c_str(),m_nServerPort);			
	}

	if (Connect(m_cmdSocket, m_strServerIP, m_nServerPort) < 0){		
		warn("!!! connect to server fail!\n");
		m_cmdSocket = INVALID_SOCKET;
		return FTP_FAIL;
	}
	
	m_strResponse = serverResponse(m_cmdSocket);
	trace("@@@@line:%d, Response: %s\n", __LINE__, m_strResponse.c_str());

	ret = parseResponse(m_strResponse);
	if(ret != 220){//ready
		warn("!!! connect to server resp fail,rsp:%d\n",ret);
		shutdown(m_cmdSocket, SHUT_RDWR);
		close(m_cmdSocket);
		m_cmdSocket = INVALID_SOCKET;
		return FTP_FAIL;
	}

	return ret;
}

FTP_API CFTPManager::inputUserName(){
	int ret = FTP_SUCC;

	if(m_strUserName.empty()){
		warn("!!! do not get valid username!\n");	
		return FTP_FAIL;
	}
	
	std::string strCommandLine = composeCommand(FTP_COMMAND_USERNAME, m_strUserName);
	if (Send(m_cmdSocket, strCommandLine) < 0){
		warn("!!! send user name to server fail!\n");
		return FTP_FAIL;
	}

	m_strResponse = serverResponse(m_cmdSocket);
	ret = parseResponse(m_strResponse);
	if(ret == 220){
		m_strResponse = serverResponse(m_cmdSocket);
		ret = parseResponse(m_strResponse);
	}
	
	trace("@@@@line:%d, Response: %s\n",  __LINE__, m_strResponse.c_str());
	if(ret != 331){//request password
		warn("!!! after input user name,rsp:%d!\n", ret);
		return FTP_FAIL;
	}

	return ret;
}

FTP_API CFTPManager::inputPassWord(){

	int ret = FTP_SUCC;
	
	if(m_strPassWord.empty()){
		warn("!!! do not get valid password!\n");	
		return FTP_FAIL;
	}
	
	std::string strCmdLine = composeCommand(FTP_COMMAND_PASSWORD, m_strPassWord);
	if (Send(m_cmdSocket, strCmdLine) < 0){
		warn("!!! send password to server fail!line:%d\n", __LINE__);
		return FTP_FAIL;
	}

	m_strResponse = serverResponse(m_cmdSocket);
	trace("@@@@line:%d, Response: %s\n",  __LINE__, m_strResponse.c_str());

	ret = parseResponse(m_strResponse);
	if(ret != 230){//login success
		warn("!!! after input password,rsp:%d!\n", ret);
		return FTP_FAIL;
	}

	m_bLogin = true;

	return ret;
}

FTP_API CFTPManager::quitServer(void){
	
	std::string strCmdLine = composeCommand(FTP_COMMAND_QUIT, "");
	if (Send(m_cmdSocket, strCmdLine) < 0){
		warn("!!! send quit command to server fail!\n");
		Close(m_cmdSocket);
		m_bLogin = false;
		
		return FTP_FAIL;
	}

	m_strResponse = serverResponse(m_cmdSocket);
	trace("@@@@line:%d, Response: %s\n",  __LINE__, m_strResponse.c_str());
	Close(m_cmdSocket);
	m_bLogin = false;
	return FTP_SUCC;
}

FTP_API CFTPManager::setTransferMode(type mode){
	std::string strCmdLine;
	int ret = FTP_SUCC;
	
	switch (mode)
	{
		case binary:
			strCmdLine = composeCommand(FTP_COMMAND_TYPE_MODE, "I");
			break;
		case ascii:
			strCmdLine = composeCommand(FTP_COMMAND_TYPE_MODE, "A");
			break;
		default:
			return FTP_FAIL;
	}

	if (Send(m_cmdSocket, strCmdLine.c_str()) < 0){
		warn("!!! send type mode cmd to server fail!\n");
		return FTP_FAIL;
	}
	
	m_strResponse  = serverResponse(m_cmdSocket);
	trace("@@@@line:%d, Response: %s\n",  __LINE__, m_strResponse.c_str());

	ret = parseResponse(m_strResponse);
	if(ret != 200){//success
		return FTP_FAIL;
	}

	return ret;
}

const std::string CFTPManager::Pasv(){
	std::string strCmdLine = composeCommand(FTP_COMMAND_PSAV_MODE, "");
	
	if (Send(m_cmdSocket, strCmdLine.c_str()) < 0){
		warn("!!! send passv cmd to server fail!\n");
		return "";
	}

	m_strResponse = serverResponse(m_cmdSocket);
	trace("@@@@line:%d, Response: %s\n",  __LINE__, m_strResponse.c_str());
	return m_strResponse;
}

const std::string CFTPManager::Port(std::string local_ip, int data_port){

	std::string buff = CompositString(local_ip, data_port);
	std::string strCmdLine = composeCommand(FTP_COMMAND_PORT_MODE, buff);
	if (Send(m_cmdSocket, strCmdLine.c_str()) < 0){
		warn("!!! send port cmd to server fail!\n");
		return "";
	}

	m_strResponse = serverResponse(m_cmdSocket);
	trace("@@@@line:%d, Response: %s\n",  __LINE__, m_strResponse.c_str());
	return m_strResponse;
}

FTP_API CFTPManager::CD(const std::string &path){
	std::string strCmdLine = composeCommand(FTP_COMMAND_CHANGE_DIRECTORY, path);

	if (Send(m_cmdSocket, strCmdLine) < 0){
		warn("!!! send cd cmd to server fail!\n");
		Close(m_cmdSocket);
		return FTP_FAIL;
	}
		
	m_strResponse = serverResponse(m_cmdSocket);
	
	trace("@@@@line: %d, Response: %s\n",  __LINE__, m_strResponse.c_str());
	return parseResponse(m_strResponse);
}

FTP_API CFTPManager::CreateDirectory(const std::string &strRemoteDir){
	
	std::string strCmdLine = composeCommand(FTP_COMMAND_CREATE_DIRECTORY, strRemoteDir);

	if (Send(m_cmdSocket, strCmdLine) < 0){
		warn("!!! send mkdir cmd to server fail!\n");
		return FTP_FAIL;
	}
	
	m_strResponse = serverResponse(m_cmdSocket);

	trace("@@@@line:%d, Response: %s\n",  __LINE__, m_strResponse.c_str());
	return parseResponse(m_strResponse);
}

FTP_API CFTPManager::Rename(const std::string &strRemoteFile, const std::string &strNewFile){
	int ret = FTP_SUCC;
	std::string strCmdLine = composeCommand(FTP_COMMAND_RENAME_BEGIN, strRemoteFile);
	if((ret = Send(m_cmdSocket, strCmdLine)) < 0){
		warn("!!! send rename from cmd fail!\n");
		return FTP_FAIL;
	}

	m_strResponse = serverResponse(m_cmdSocket);
	trace("@@@@line:%d, Response: %s\n",  __LINE__, m_strResponse.c_str());	
	ret = parseResponse(m_strResponse);
	if(ret != 350){
		if(m_strResponse.find("350") == m_strResponse.npos){
			m_strResponse = serverResponse(m_cmdSocket);
			if(m_strResponse.find("350") == m_strResponse.npos){
				warn("!!! rename from cmd fail,rsp:%d!\n",ret);
				return FTP_FAIL;
			}
		}
	}

	if((ret = Send(m_cmdSocket, composeCommand(FTP_COMMAND_RENAME_END, strNewFile))) < 0){
		warn("!!! send rename name to cmd fail!\n");
		return FTP_FAIL;		
	}

	m_strResponse = serverResponse(m_cmdSocket);
	trace("@@@@line: %d, Response: %s\n",  __LINE__, m_strResponse.c_str());
	ret = parseResponse(m_strResponse);
	if(ret != 250){
		warn("!!! rename to cmd fail,rsp:%d!\n",ret);
		return FTP_FAIL;
	}

	return FTP_SUCC;
}

	const std::string CFTPManager::Nlist(const std::string &path, int cmd_type){

	int ret = FTP_SUCC;
	int data_fd = INVALID_SOCKET;

	data_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(data_fd < 0){
		warn("!!! list cmd try create data socket fail!\n");
		return "";
	}

	//set port reused
	int addruse = 1;
	setsockopt(data_fd, SOL_SOCKET, SO_REUSEADDR, &addruse, sizeof(addruse));

	if (createDataLink(data_fd) < 0){
		warn("!!! list CreateDataLink fail!\n");
		Close(data_fd);
		return "";
	}
	
	std::string strCmdLine = composeCommand(cmd_type, path);
	if (Send(m_cmdSocket, strCmdLine) < 0){
		Close(data_fd);
		warn("!!! send list cmd to server fail!\n");
		return "";
	}

	m_strResponse = serverResponse(m_cmdSocket);
	trace("@@@@line: %d, Response: %s\n",  __LINE__, m_strResponse.c_str());
	ret = parseResponse(m_strResponse);
	if((ret != 150) && (ret != 125)){
		Close(data_fd);
		warn("!!! lis command fail,rsp:%d!\n",ret);
		return "";
	}

	char data[16*1024] = {0};
	ret = getData(data_fd, data, 16*1024);
	if(ret > 0){
		Close(data_fd);

		m_strResponse = serverResponse(m_cmdSocket);
		trace("@@@@line: %d, Response: %s\n",  __LINE__, m_strResponse.c_str());
		
		std::string list_rsp = data;
		return list_rsp;
	}

	warn("!!! list cmd get data fail!\n");
	Close(data_fd);

	m_strResponse = serverResponse(m_cmdSocket);
	trace("@@@@line: %d, Response: %s\n",  __LINE__, m_strResponse.c_str());

	return "";
}

FTP_API CFTPManager::DelTemps(std::string path){
#if USE_LIST
	string list_rsp  = Nlist(path, FTP_COMMAND_DIR);
	if(list_rsp.empty()){
		return FTP_SUCC;
	}
	
	while(1)  {  
		int nIdx = list_rsp.find("\r\n");  
		if(nIdx == -1){
			break;
		}

		string strListDataTemp = list_rsp.substr(0 , nIdx);
		if(strListDataTemp.at(0) == 'd'){//dir
			list_rsp = list_rsp.substr(nIdx + 2, list_rsp.length() - nIdx - 2);
			continue;  
		}  

		int i = 0;  
		while(1)  {
			int npos = strListDataTemp.find(" ");  
			strListDataTemp = strListDataTemp.substr(npos + 1, strListDataTemp.length() - npos - 1);
			strListDataTemp.erase(0,strListDataTemp.find_first_not_of(' ')); 
			strListDataTemp.erase(0,strListDataTemp.find_first_not_of('\t')); 

			if(++i == 8)
			break;
		}  

		int tpos = strListDataTemp.find(".tmp"); 
		if(tpos > 0){
			DelFile(strListDataTemp);
		}

		list_rsp = list_rsp.substr(nIdx + 2, list_rsp.length() - nIdx - 2);
	} 
#else
	string list_rsp  = Nlist(path, FTP_COMMAND_NLIST);
		if(list_rsp.empty()){
		return FTP_SUCC;
	}

	while(1) {
		int nIdx = list_rsp.find("\r\n");
		if(nIdx == -1){
			break;
		}

		string strListDataTemp = list_rsp.substr(0 , nIdx);
		int tpos = strListDataTemp.find(".tmp");
		if(tpos > 0){
			DelFile(strListDataTemp);
		}

		list_rsp = list_rsp.substr(nIdx + 2, list_rsp.length() - nIdx - 2);
	}
#endif
	return FTP_SUCC;
}

FTP_API CFTPManager::DelFile(const std::string &strRemoteFile){
	int ret = FTP_SUCC;
	printf("start delete tmp file:%s\n", strRemoteFile.c_str());
	std::string strCmdLine = composeCommand(FTP_COMMAND_DELETE_FILE, strRemoteFile);
	if((ret = Send(m_cmdSocket, strCmdLine)) < 0){
		warn("!!! send dele cmd fail!\n");
		return FTP_FAIL;
	}

	m_strResponse = serverResponse(m_cmdSocket);
	trace("@@@@line:%d, Response: %s\n",  __LINE__, m_strResponse.c_str());

	ret = parseResponse(m_strResponse);
	if(ret != 250){
		warn("!!! dele command fail,rsp:%d!\n",ret);
		return FTP_FAIL;
	}

	return ret;
}

long CFTPManager::GetFileLen(const std::string &strRemoteFile){

	int ret = FTP_SUCC;
	std::string strCmdLine = composeCommand(FTP_COMMAND_FILE_SIZE, strRemoteFile);  

	if (Send(m_cmdSocket, strCmdLine) < 0)  {
		warn("!!! send size cmd fail!\n");
		return FTP_FAIL;
	}  
  
	m_strResponse = serverResponse(m_cmdSocket);  
	trace("@@@@Response: %s\n", m_strResponse.c_str());  

	ret = parseResponse(m_strResponse);
	if (ret == 213) {  
		std::string strData = m_strResponse.substr(4);  
		trace("@@@file len return string: %s\n", strData.c_str());  
		unsigned long len = atol(strData.c_str());  

		return len;  
	}  

	return FTP_FAIL;  
} 

void CFTPManager::Close(int &sock){
	shutdown(sock, SHUT_RDWR);
	close(sock);
	sock = INVALID_SOCKET;
}

FTP_API CFTPManager::Push(std::string FilePrefix, char *data, int size){

	std::string strCmdLine;
	int ret = FTP_SUCC;

	if(data == NULL || size <= 0){
		return FTP_FAIL;
	}

	trace("@@@ push file:%s\n", FilePrefix.c_str());
	ret = connect2Server();
	if(ret < 0){ 
		warn("!!! ftp connect to server fail!\n");
		return FTP_FAIL;
	}

	ret = inputUserName();
	if(ret < 0){
		warn("!!! ftp cmd input username fail!\n");
		quitServer();
		return FTP_FAIL;
	}

	ret = inputPassWord();
	if( ret < 0){
		warn("!!! ftp cmd input password fail!\n");
		quitServer();
		return FTP_FAIL;
	}

	ret = setTransferMode(binary);
	if(ret < 0){
		warn("!!! ftp cmd set to binary mode fail!\n");
		quitServer();
		return FTP_FAIL;
	}

	if(!m_strPath.empty()){
		printf("@@@start create dst dir:%s\n",m_strPath.c_str());
		std::list<std::string> strArray ;
		std::list<std::string>::iterator ite;

		SplitString(m_strPath , strArray , "/" );
		ite = strArray.begin();
		while(ite != strArray.end()){
			if(((std::string)*ite).empty()){
				break;
			}

			ret = CD(*ite);
			if(ret < 0){
				warn("!!! ftp cmd change dir fail!dir:%s\n", ((std::string)*ite).c_str());
				quitServer();
				return FTP_FAIL;
			}
			
			if(ret == 550){//file can not access
				ret = CreateDirectory(*ite);
				if(ret < 0 || ret != 257){//create success
					warn("!!! ftp cmd create dir fail!dir:%s\n", ((std::string)*ite).c_str());
					quitServer();
					return FTP_FAIL;
				}

				ret = CD(*ite);
				if(ret < 0 || ret != 250){//change success
					warn("!!! ftp change dir fail!dir:%s\n", ((std::string)*ite).c_str());
					quitServer();
					return FTP_FAIL;
				}
			}
			
			ite++;
			}
	}

	if(cycle_cout == 100){
		DelTemps("./"); //delete old tmp files
		cycle_cout = 0;
	}
	cycle_cout++;

	int retry_times = 0;
	int data_fd = INVALID_SOCKET;
D_RETRY:
	data_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(data_fd < 0){
		retry_times++;
		if(retry_times > 3){
			warn("!!! try create data socket too much times, quit!\n");
			quitServer();
			return FTP_FAIL;
		}
		
		goto D_RETRY;
	}
	
	if (createDataLink(data_fd) < 0){
		warn("!!! CreateDataLink fail!\n");
		Close(data_fd);
		
		quitServer();
		return FTP_FAIL;
	}

	string processing_file = FilePrefix + ".tmp";
	strCmdLine = composeCommand(FTP_COMMAND_UPLOAD_FILE, processing_file);
	if (Send(m_cmdSocket, strCmdLine) < 0){
		warn("!!! send upload cmd to server fail!line:%d\n", __LINE__);
		Close(data_fd);
		m_strResponse = serverResponse(m_cmdSocket);
		trace("@@@@line:%d, Response: %s\n",  __LINE__, m_strResponse.c_str());
		
		quitServer();
		return FTP_FAIL;
	}
	trace("@@@@line: %d, Response: %s\n",  __LINE__, serverResponse(m_cmdSocket).c_str());
	ret = parseResponse(m_strResponse);
	if((ret != 150) && (ret != 125)){
		warn("!!! upload cmd rsp fail!rsp:%d\n", ret);
		Close(data_fd);
		m_strResponse = serverResponse(m_cmdSocket);
		trace("@@@@line:%d, Response: %s\n",  __LINE__, m_strResponse.c_str());

		quitServer();
		return FTP_FAIL;
	}
	
	if (Send(data_fd, (char *)data, size) < 0){
		warn("!!! send data to server fail!line:%d\n", __LINE__);

		Close(data_fd);
		m_strResponse = serverResponse(m_cmdSocket);
		trace("@@@@line:%d, Response: %s\n",  __LINE__, m_strResponse.c_str());
		
		quitServer();
		return FTP_FAIL;
	}
	
	Close(data_fd);
	trace("@@@@line: %d, Response: %s\n",  __LINE__, serverResponse(m_cmdSocket).c_str());
	ret = parseResponse(m_strResponse);
	if(ret != 226){
		warn("!!! close data link rsp fail!rsp:%d\n", ret);
	}

	string final_file = FilePrefix + ".jpg";
	int rename_cnt = 0;
Rename_retry:
	ret = Rename(processing_file, final_file);
	if(ret < 0){
		rename_cnt++;
		if(rename_cnt > 2){
			warn("!!! rename fail retry limited,quit!\n");

			quitServer();
			return FTP_FAIL;
		}

		goto Rename_retry;
	}

	quitServer();
	return size;
}

const std::string CFTPManager::composeCommand(const unsigned int command, const std::string &strParam){
	if (command < FTP_COMMAND_BASE || command > FTP_COMMAND_END){
		return "";
	}

	std::string strCommandLine;

	m_nCurrentCommand = command;
	m_commandStr.clear();

	switch (command)
	{
	case FTP_COMMAND_USERNAME:
		strCommandLine = "USER ";
		break;
	case FTP_COMMAND_PASSWORD:
		strCommandLine = "PASS ";
		break;
	case FTP_COMMAND_QUIT:
		strCommandLine = "QUIT ";
		break;
	case FTP_COMMAND_CURRENT_PATH:
		strCommandLine = "PWD ";
		break;
	case FTP_COMMAND_TYPE_MODE:
		strCommandLine = "TYPE ";
		break;
	case FTP_COMMAND_PSAV_MODE:
		strCommandLine = "PASV ";
		break;
	case FTP_COMMAND_DIR:
		strCommandLine = "LIST ";
		break;
	case FTP_COMMAND_CHANGE_DIRECTORY:
		strCommandLine = "CWD ";
		break;
	case FTP_COMMAND_DELETE_FILE:
		strCommandLine = "DELE ";
		break;
	case FTP_COMMAND_DELETE_DIRECTORY:
		strCommandLine = "RMD ";
		break;
	case FTP_COMMAND_CREATE_DIRECTORY:
		strCommandLine = "MKD ";
		break;
	case FTP_COMMAND_RENAME_BEGIN:
		strCommandLine = "RNFR ";
		break;
	case FTP_COMMAND_RENAME_END:
		strCommandLine = "RNTO ";
		break;
	case FTP_COMMAND_FILE_SIZE:
		strCommandLine = "SIZE ";
		break;
	case FTP_COMMAND_DOWNLOAD_FILE:
		strCommandLine = "RETR ";
		break;
	case FTP_COMMAND_DOWNLOAD_POS:
		strCommandLine = "REST ";
		break;
	case FTP_COMMAND_UPLOAD_FILE:
		strCommandLine = "STOR ";
		break;
	case FTP_COMMAND_APPEND_FILE:
		strCommandLine = "APPE ";
		break;
	case FTP_COMMAND_PORT_MODE:
		strCommandLine = "PORT ";
		break;
	case FTP_COMMAND_NOOP:
		strCommandLine = "NOOP ";
		break;			
	case FTP_COMMAND_NLIST:
		strCommandLine = "NLST ";
		break;

	default :
		break;
	}

	strCommandLine += strParam;
	strCommandLine += "\r\n";

	m_commandStr = strCommandLine;
	trace("@@@composeCommand: %s", m_commandStr.c_str());

	return m_commandStr;
}

FTP_API CFTPManager::Connect(int socketfd, const std::string &serverIP, unsigned int nPort){
	if (socketfd == INVALID_SOCKET){
		return FTP_FAIL;
	}

	unsigned long ul = 1;
	ioctl(socketfd, FIONBIO, &ul); //设置为非阻塞模式,否则ftp地址不存在时,connect会阻塞

	int error = FTP_FAIL;
	int len = sizeof(int);
	struct sockaddr_in  addr;
	bool ret = false;
	timeval stime;
	fd_set  set;

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port	= htons(nPort);
	addr.sin_addr.s_addr = inet_addr(serverIP.c_str());
	bzero(&(addr.sin_zero), 8);

	trace("Address: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	if (connect(socketfd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) == -1){   //若直接返回 则说明正在进行TCP三次握手
	
		stime.tv_sec = 2; //设置超时时间
		stime.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(socketfd, &set);

		if (select(socketfd + 1, NULL, &set, NULL, &stime) > 0){   ///在这边等待 阻塞 返回可以读的描述符 或者超时返回0  或者出错返回-1
			getsockopt(socketfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t*)&len);
			if (error == 0){
				ret = true;
			} else {
				warn("!!! socket connect error:%d\n", error);
				ret = false;
			}
		}
	} else {
		trace("Connect Immediately!!!\n");
		ret = true;
	}

	if (!ret){
		shutdown(socketfd, SHUT_RDWR);
		close(socketfd);
		warn("!!! cannot connect server!!\n");
		return FTP_FAIL;
	}

	// use linger, limit the waiting time when close ftp(not neccessory)
	struct linger stLinger = { 1, 2 };
	struct linger {
		int l_onoff; /* 0 denotes that linger is disabled, else means enabled */
		int l_linger; /* linger time, in second. the value must be >= 2 */
	};
	/*
	* linger :徘徊的意思。SO_LINGER:表示经历time_wait阶段，且时间是stLinger中第二个参数指定的值。
	* 当套接口关闭时内核将拖延一段时间（由l_linger决定）。如果套接口缓冲区中仍残留数据，进程将处于睡眠状态，直到
	* （a）所有数据发送完且被对方确认，之后进行正常的终止序列（描述字访问计数为0）或
	* （b）延迟时间到。
	* */
	int flags = setsockopt(socketfd, SOL_SOCKET, SO_LINGER, &stLinger, sizeof(struct linger));
	if (flags == FTP_FAIL)
	{
		shutdown(socketfd, SHUT_RDWR);
		close(socketfd);
		return FTP_FAIL;
	}

	return FTP_SUCC;
}


const std::string CFTPManager::serverResponse(int sockfd){
	if (sockfd == INVALID_SOCKET){
		return "";
	}
	
	int nRet = FTP_FAIL;
	char buf[MAX_PATH] = {0};
	m_strResponse.clear();

	if((nRet = getData(sockfd, buf, MAX_PATH)) > 0){
		buf[MAX_PATH - 1] = '\0';
		m_strResponse = buf;
		return m_strResponse;
	}

	warn("!!! socket getData fail!\n");
	return "";
}

FTP_API CFTPManager::getData(int fd, char *strBuf, unsigned long length){

	timeval stime;
	fd_set	readfd;
	int nLen = 0;
	int ret = FTP_SUCC;
	
	if(strBuf == NULL){
		warn("!!! socket rcv buff is NULL!\n");
		return FTP_FAIL;
	}

	if (fd <= 0){
		warn("!!! socket fd is invalid!\n");
		return FTP_FAIL;
	}
	memset(strBuf, 0, length);

	int retry_cnt = 0;
	stime.tv_sec = 1;
	stime.tv_usec = 0;

Rcv_Retry:
	FD_ZERO( &readfd );
	FD_SET(fd, &readfd );
	ret = select(fd + 1, &readfd, 0, 0, &stime);
	if (ret > 0){
		if ((nLen = recv(fd, strBuf, length, 0)) > 0){
			return nLen;
		} else {
			warn("!!! socket rcv none data or error! ret:%d\n",nLen);
			return FTP_FAIL;
		}
	} else if (ret < 0){
		warn("!!! socket rcv mode select error!\n");
		return FTP_FAIL;
	} else {
		retry_cnt++;
		if(retry_cnt <= 3){
			warn("!!! socket rcv retry!\n");
			goto Rcv_Retry;
		}
	}
	
	return FTP_FAIL;
}

FTP_API CFTPManager::Send(int fd, const std::string &cmd){

	if (fd <= 0){
		return FTP_FAIL;
	}
	
	return Send(fd, cmd.c_str(), cmd.length());
}

FTP_API CFTPManager::Send(int fd, const char *data, const size_t len){

	int ret = FTP_SUCC;
	fd_set  writefd;
	timeval timeout;
	timeout.tv_sec  = 2;
	timeout.tv_usec = 0;

	if((FTP_COMMAND_USERNAME != m_nCurrentCommand) 
		&&(FTP_COMMAND_PASSWORD != m_nCurrentCommand)
		&&(!m_bLogin)){
		warn("!!! has not login,can not send data!\n");
		return FTP_FAIL;
	}

	trace("@@@ socket need send:%d\n", len);
	size_t nlen = len;
	while(nlen > 0){
		FD_ZERO(&writefd);  
		FD_SET(fd, &writefd);
		ret = select(fd + 1, 0, &writefd , 0 , &timeout);
		if(ret > 0){
			int nSendLen = send(fd, data , (int)nlen , 0);
			trace("@@@ socket send return %d\n", nSendLen);
			if(nSendLen < 0){
				warn("!!! socket send return error! return value:%d\n", nSendLen);
				return FTP_FAIL; 
			}
			
			nlen = nlen - nSendLen;
			data +=  nSendLen;
		} else if(ret < 0){
			warn("!!! socket send select error!");
			return FTP_FAIL;
		} else {
			warn("!!! socket send select timeout!\n");
			continue;
		}
	}

	return FTP_SUCC;
}

FTP_API CFTPManager::createDataLink(int data_fd){
	int ret = FTP_SUCC;
	
	if(data_fd <= 0){
		return FTP_FAIL;
	}

	std::string strData;
	unsigned long nPort = 0 ;
	std::string strServerIp ; 
	std::list<std::string> strArray ;

	std::string parseStr = Pasv();
	if (parseStr.size() <= 0){
		warn("!!! pasv return null response!\n");
		return FTP_FAIL;
	}

	ret = parseResponse(parseStr);
	if(ret == 421){
		warn("!!! passive cmd return timeout!\n");
		return FTP_FAIL;
	}
	//trace("parseInfo: %s\n", parseStr.c_str());

	size_t nBegin = parseStr.find_first_of("(");
	size_t nEnd	  = parseStr.find_first_of(")");
	strData		  = parseStr.substr(nBegin + 1, nEnd - nBegin - 1);

	//trace("ParseAfter: %s\n", strData.c_str());
	if( SplitString( strData , strArray , "," ) <0)
		return FTP_FAIL;

	if( ParseString( strArray , nPort , strServerIp) < 0)
		return FTP_FAIL;

	//trace("nPort: %ld IP: %s\n", nPort, strServerIp.c_str());
	if (Connect(data_fd, m_strServerIP, nPort) < 0){
		warn("!!! data link connect fail!\n");
		return FTP_FAIL;
	}

	return FTP_SUCC;
}

FTP_API CFTPManager::createDataLink(int data_fd, int accp_fd){
	int ret = FTP_SUCC;
	struct sockaddr_in sckaddr;
	int len = 0; 

	string local_ip;
	int local_port = 0;

	timeval stime;
	fd_set	waitfd;

	if(data_fd <= 0){
		return FTP_FAIL;
	}

	sckaddr.sin_family = AF_INET;
	sckaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sckaddr.sin_port = 0;
	len = sizeof(sckaddr);
	ret = ::bind(data_fd, (sockaddr* )&sckaddr, len);
	if(ret < 0){
		warn("!!! port mode socket bind error!\n");
		return FTP_FAIL;
	}

	ret = listen(data_fd, 1);
	if(ret < 0){
		warn("!!! port mode socket linsten error!\n");
		return FTP_FAIL;
	}

	len = sizeof(sckaddr);
	ret = getsockname(data_fd, (sockaddr *)&sckaddr, (socklen_t*)&len); 
	if(ret < 0){
		warn("!!! port mode getsockname error!\n");
		return FTP_FAIL;
	}

	local_ip = GetLocalIp();
	if(local_ip.length() <=0){
		warn("!!! port mode do not get valid addr!\n");
		return FTP_FAIL;
	}
	local_port = htons(sckaddr.sin_port) + 1;

	std::string parseStr = Port(local_ip, local_port);
	ret = parseResponse(parseStr);
	if(ret != 200){
		warn("!!! port cmd return error!\n");
		return FTP_FAIL;
	}
	
	stime.tv_sec = 4;
	stime.tv_usec = 0;
	FD_ZERO( &waitfd );
	FD_SET(data_fd, &waitfd );
	ret = select(data_fd + 1, &waitfd, 0, 0, &stime);
	if (ret > 0){
		accp_fd = accept(data_fd, (struct sockaddr *)NULL, NULL);
		if(accp_fd <= 0){
			warn("!!! passive cmd return timeout!\n");
			return FTP_FAIL;
		}
	} else {
		warn("!!! socket rcv mode select error!\n");
		return FTP_FAIL;
	}

	return FTP_SUCC;
}

FTP_API CFTPManager::parseResponse(const std::string &str){
	if(str.empty()){
		return FTP_FAIL;
	}
	
	std::string strData = str.substr(0, 3);
	unsigned int val = atoi(strData.c_str());

	return val;
}

