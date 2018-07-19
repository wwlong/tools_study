#include <stdio.h>
//shm进程间通信接口协议
//进程间通信基于共享内存shm（Linux SHared Memory）。共享内存中存放一个预定义的数据结构，配合读写完成的标志，来协调两端的访问。
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>

#define DEEPGLINT_YUV_STREAM_CNT (3)
#define DEEPGLINT_SHM_NAME "deepglint" //共享内存名称
#define DEEPGLINT_DAYLIGHT_MODE_DAYLIGHT (0)	//设备日夜模式
#define DEEPGLINT_DAYLIGHT_MODE_NIGHT (1)	//设备夜模式
#define DEEPGLINT_MD_OFF (0)	//移动侦测状态
#define DEEPGLINT_MD_ON (1)		//移动侦测状态
typedef struct _deepglint_devinfo
{
	int readDone;//读完成标志位:客户端读完成后需要置1
	int writeDone;//写完成标志位:服务端写完成后置1
	char devModel[64];//设备型号
	char devMagic[64];//设备唯一号
	char devSn[64];//设备序列号， 后10位是易视网ID
	int daylightMode;//设备日夜模式
}ST_DEEPGLINT_DEVINFO, *LP_DEEPGLINT_DEVINFO;
typedef struct _deepglint_event
{
	int readDone;//读完成标志位:客户端读完成后需要置1
	int writeDone;//写完成标志位:服务端写完成后置1
	int motion_detect;//移动侦测标记位:DEEPGLINT_MD_ON为有移动侦测
}ST_DEEPGLINT_EVENT, *LP_DEEPGLINT_EVENT;
typedef struct _deepglint_yuv
{
	int readDone;//读完成标志位:客户端读完成后需要置1
	int writeDone;//写完成标志位:服务端写完成后置1
	char yuvSdkInfo[256]; //海思 VIDEO_FRAME_INFO_S 结构体
}ST_DEEPGLINT_YUV, *LP_DEEPGLINT_YUV;
//共享内存数据结构
typedef struct _deepglint_info
{
	ST_DEEPGLINT_DEVINFO devInfo;
	ST_DEEPGLINT_EVENT event;
	ST_DEEPGLINT_YUV yuvInfo[3];
}ST_DEEPGLINT_INFO, *LP_DEEPGLINT_INFO;
int main(int argc, char** argv)
{
	int fd = 0, i = 0;
	LP_DEEPGLINT_INFO info;
	//open shm
	fd = shm_open(DEEPGLINT_SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0){
		perror("shm_open error\r\n");
		return -1;
	}
	//set shm size
	ftruncate(fd, sizeof(ST_DEEPGLINT_INFO));
	info = (LP_DEEPGLINT_INFO)mmap(NULL, sizeof(ST_DEEPGLINT_INFO), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(info, 0, sizeof(ST_DEEPGLINT_INFO));
    int cnt = 0;
	if(info != NULL){
		while(1){
            //设置info信息
            memcpy(info->devInfo.devModel, "dg", 2);
            memcpy(info->devInfo.devMagic, "magic", 5);
            memcpy(info->devInfo.devSn, "devsn", 5);
			info->devInfo.daylightMode = cnt ++;
            info->event.writeDone = 1;

            printf("model:%s\n", info->devInfo.devModel);
			printf("magic:%s\n", info->devInfo.devMagic);
			printf("sn:%s\n", info->devInfo.devSn);
			printf("daylightmode:%d\n", info->devInfo.daylightMode);	

			sleep(1);
		}
 
		munmap((void *)info, sizeof(ST_DEEPGLINT_INFO));
		shm_unlink(DEEPGLINT_SHM_NAME);
	}
	return 0;
}




