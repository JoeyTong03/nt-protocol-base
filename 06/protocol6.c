/* protocol6.c */
#include "../common/common.h"
#include "protocol6.h"

/**********************
* 函数名称：network_write
* 功    能：网络层捕捉到SIG_ENABLE_NETWORK信号后，向数据链路层传递数据
* 参    数：buffer--获取到的数据包;sharedFilePath---共享文件路径
* 返    回：Status: 0--文件打开失败
* 说    明：供xxx_datalink进程使用，xxx与上一致；使用文件锁机制
***********************/
void network_write(int sig)
{
	char sharedFilePath[PATHLENGTH];//共享文件名/路径network_datalink.share.xxxx
	int fd;//共享文件描述符
	char buf[MAX_PKT];//读取的数据
	int perCount=1024;
	int tmp_isEnd=0;
	if(datacount<=TOTALDATASIZE)//
	{
		if(TOTALDATASIZE-datacount<MAX_PKT)//最后一个数据报不足1024字节
			perCount=TOTALDATASIZE-datacount;
		readSendFile(buf,perCount);//从1G文件中每次读取perCount字节
		datacount+=1024;
	}
	else
	{
		memset(buf,'\0',MAX_PKT);//作为结束标志的全尾零文件标志
		tmp_isEnd=1;
	}
		
	getSharedFilePath(count,sharedFilePath);
	fd=open(sharedFilePath,O_CREAT | O_WRONLY,0666);
	if(fd<0)//文件创建/打开失败
	{
		printf("Shared file %s open fail!\n",sharedFilePath);
		return ;
	}
	/*为共享文件加写锁*/
	if(lock_set(fd,F_WRLCK)==FALSE)//上写锁失败
		exit(-1); 
		//continue;
	
	/*向共享文件中写入要传递的数据*/
	if(write(fd,buf,MAX_PKT)<0)
	{
		printf("Write share file %s fail!\n",sharedFilePath);
		lock_set(fd,F_UNLCK);//退出前先解锁
		exit(-1);
		//continue;
	}
	lock_set(fd,F_UNLCK);//退出前先解锁
	close(fd);
	enable_datalink_read();//通知数据链路层读数据
	count++;
	if(count>999)
		count=1;//轮询使用
	if(tmp_isEnd)
		isEnd=tmp_isEnd;	
}

 /**********************
* 函数名称：readSendFile
* 功    能：发送方从1G文件中每次读取perCount字节
* 参    数：buf--读取的数据 perCount--此次读取的数据
* 返    回：
* 说    明：不足1024字节，用\0填充
***********************/
void readSendFile(char buf[],int perCount)
{
	int fd;
	fd=open(SEND_FILE,O_RDONLY,0666);
	if(fd==-1)
	{
		printf("Data file open fail!\n");
		return;
	}
	if(read(fd,buf,perCount)<0)
	{
		printf("Data file read fail!\n");
		return;
	}
	if(perCount<MAX_PKT)
	{
		int i=perCount;
		for(;i<MAX_PKT;i++)
			buf[i]='\0';//不足1024字节，用\0填充
	}
}

 /**********************
* 函数名称：between
* 功    能：判断接收帧b是否落在接收窗口内
* 参    数：a--窗口下界  b--接收帧  c--上界
* 返    回：在--1   不在--0
* 说    明：
***********************/
static int between(seq_nr a,seq_nr b,seq_nr c)
{
	if((a<=b)&&(b<c) || (c<a)&&(a<=b) || (b<c)&&(c<a))
		return 1;
	return 0;
}


 /**********************
* 函数名称：send_data
* 功    能：发送帧到物理层
* 参    数：fk--帧的类型 frame_nr--发送的帧序号  
			frame_expected--等待接收的帧序号
			buffer---存放帧的缓冲区
* 返    回：
* 说    明：
***********************/
static void send_data(frame_kind fk,seq_nr frame_nr,seq_nr frame_expected,Packet buffer[])
{
	int i;
	
	Frame s;
	s.kind=fk;
	if(fk==data)
		for(i=0;i<MAX_PKT;i++)
			(s.info.data)[i]=buffer[frame_nr%NR_BUFS][i];
	s.seq=frame_nr;
	s.ack=(frame_expected+MAX_SEQ)%(MAX_SEQ+1);
	if(fk==nak)
		no_nak=FALSE;
	to_physical_layer(&s);
	if(fk==data)
		start_timer(frame_nr%NR_BUFS);
	stop_ack_timer();
}


void protocol6(void)
{
	seq_nr ack_expected;
	seq_nr next_frame_to_send;
	seq_nr frame_expected;
	seq_nr too_far;
	int i;
	frame r;
	Packet out_buf[NR_BUFS];
	Packet in_buf[NR_BUFS];
	int arrived[NR_BUFS];
	seq_nr nbuffered;
	event_type event;
	
	enable_network_layer();
	ack_expected=0;
	next_frame_to_send=0;//初始帧号为0
	frame_expected=0; //希望收到的帧号
	too_far=NR_BUFS;//初始为非法序号
	nbuffered=0;
	for(i=0;i<NR_BUFS;i++)
		arrived[i]=FALSE;
	
	while(1)
	{
		wait_for_event(&event);
		switch(event)
		{
			case network_layer_ready:
			{
				nbuffered=nbuffered+1;
				from_network_layer(&out_buf[next_frame_to_send%NR_BUFS]);
				send_data(data,next_frame_to_send,frame_expected,out_buf);
				inc(next_frame_to_send);
				break;
			}
			case frame_arrival:
			{
				
			}
			case cksum_err:
			{
				
			}
			case timeout:
			{
				
			}
			case ack_timeout:
			{
				
			}
		}
		if(nbuffered<NR_BUFS)
			enable_network_layer();
		else
			disable_network_layer();
	}
}