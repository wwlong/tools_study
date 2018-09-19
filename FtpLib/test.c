#include "stdio.h"
#include <memory>
#include "FtpClientLib.h"


int main()
{

	FTPParams ftp_params;
	ftp_params.ip = "172.17.0.102";
        ftp_params.port = 21;
        ftp_params.username = "ftp";
        ftp_params.password = "1";	
	ftp_params.path = "data";

	FILE *rdfile = fopen("./1234.jpg", "rb");
	fseek(rdfile,0,SEEK_END); //��λ���ļ�ĩ
	int nFileLen = ftell(rdfile); //�ļ����� 

	fseek(rdfile,0,SEEK_SET);

	char *data = (char *)malloc(nFileLen);
	int ret = fread(data, 1, nFileLen, rdfile);
	if(ret != nFileLen){
		printf("read fail!\n");
	}

	CFTPManager *ftp = new CFTPManager(ftp_params);
	ftp->Push("1234", data, ret);

	free(data);
	fclose(rdfile);

        delete ftp;

	return 0;
}
