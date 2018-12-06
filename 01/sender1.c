#include "../common/common.h"
int tjindex=1; //发送文件的数量
/**********************
* 函数名称：network_write
* 功    能：网络层接收到enable_network_layer信号后的处理函数，
			生成1024字节写入共享文件，完成网络层到数据链路层的数据传递
* 参    数：检测到的信号
* 返    回：
* 说    明：
	(1)供xxx_network进程使用,xxx与上一致
	(2)使用共享文件+信号实现进程间数据传递
***********************/
void network_write(int sig)
{
	char sharedFilePath[PATHLENGTH];//共享文件名/路径network_datalink.share.xxxx
	int fd;//共享文件描述符
	char buf[BUFSIZE];//生成的数据
	if(tjindex<=5)//暂时只发5份，方便观察
	{
		getSharedFilePath(tjindex,sharedFilePath);
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
		
		/*生成1024字节数据*/
		generateData(buf);
		
		/*向共享文件中写入要传递的数据*/
		if(write(fd,buf,BUFSIZE)<0)
		{
			printf("Write share file %s fail!\n",sharedFilePath);
			lock_set(fd,F_UNLCK);//退出前先解锁
			exit(-1);
			//continue;
		}
		lock_set(fd,F_UNLCK);//退出前先解锁
		close(fd);
		enable_datalink_read();//通知数据链路层读数据
		tjindex++;
	}
}
/**********************
* 函数名称：datalink_read
* 功    能：数据链路层接收到SIG_DATALINK_RD信号后的处理函数，
			从对应的共享文件中读取网络层传递的数据
* 参    数：检测到的信号
* 返    回：
* 说    明：
	(1)供xxx_datalink进程使用,xxx与上一致
	(2)使用共享文件+信号实现进程间数据传递
***********************/
void datalink_read(int sig)
{
	Frame s;
	Packet buffer;
	char sharedFilePath[PATHLENGTH];//共享文件名/路径network_datalink.share.xxxx
	if(tjindex<=5)
	{
		getSharedFilePath(tjindex,sharedFilePath);
		
		/* 从网络层获取数据，通过共享文件+信号方式 */
		from_network_layer(&buffer,sharedFilePath);
		
		/* 将从网络层获取的数据包“装入”数据帧 */
		int i=0;
		for(i=0;i<BUFSIZE;i++)
			(s.info.data)[i]=(buffer.data)[i];
		
		/* 将数据帧传递给物理层 */
		to_physical_layer(&s);
		
		enable_network_write();//通知网络层写数据
		tjindex++;
	}
	
}

int main()
{
	pid_t nt_pid;//网络层进程的pid号
	pid_t dl_pid;//数据链路层进程的pid号
	pid_t ps_pid;//物理层进程的pid号
	while((nt_pid=fork())<0)
		sleep(1);
	
	if(nt_pid==0)//网络层进程
	{
		/* 修改进程名，便于根据进程名获得pid值，发送信号 */
		char new_name[20] = "sender1_network"; 
		prctl(PR_SET_NAME, new_name);
		
		/* 等待数据链路层读完共享文件数据后发“写”信号，随机生成1024字节数据写入共享文件
			实现网络层到数据链路层的数据传递*/
		(void) signal(SIG_NETWORK_WR,network_write);
		
		while(1)
			sleep(1);
		exit(0);
	}
	else if(nt_pid>0)//父进程fork子进程--数据链路层
	{
		while((dl_pid=fork())<0)
			sleep(1);
		if(dl_pid==0)//数据链路层进程
		{
			/* 修改进程名，便于根据进程名获得pid值，发送信号 */
			char new_name[20] = "sender1_dtlink";
			prctl(PR_SET_NAME, new_name);
			
			enable_network_write();//先通知网络层写数据
			(void) signal(SIG_DATALINK_RD,datalink_read);
			
			while(1)
				sleep(1);
			exit(0);			
		}
		else if(dl_pid>0)//父进程fork子进程--物理层
		{
			while((ps_pid=fork())<0)
				sleep(1);
			if(ps_pid==0)//物理层进程
			{
				/* 修改进程名，便于根据进程名获得pid值，发送信号 */
				char new_name[20] = "sender1_physic";
				prctl(PR_SET_NAME, new_name);
				
				Frame s;
				Packet buffer;
				int fd;
				char testPath[PATHLENGTH];
				int tjindex=1;
				while(1)
				{
					/* 从数据链路层通过有名管道获取数据帧 */
					physical_layer_from_datalink(&s);
					
					/* 为检查单侧通信，暂时写入文件便于观察，后续需要修改为socket发送到receiver端 */
					getTestPath(tjindex,testPath);
					fd=open(testPath,O_CREAT | O_WRONLY ,0666);
					if(fd<0)//文件创建/打开失败
					{
						printf("Test file %s open fail!\n",testPath);
						continue;
					}
					if(write(fd,&s,FRAMESIZE)<0)
					{
						printf("Write test file %s fail!\n",testPath);
						exit(-1);
						//continue;
					}
					
					tjindex++;
					if(tjindex>5)//暂时只发5份，方便观察
						break;
				}
				while(1)
					sleep(1);
				exit(0);
			}
			else if(ps_pid>0)//父进程
				exit(0);	
		}
	}
}

