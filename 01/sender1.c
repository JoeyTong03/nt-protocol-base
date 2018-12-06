#include "../common/common.h"

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
		
		int index=1; //第index份发送的数据
		char sharedFilePath[PATHLENGTH];//共享文件名/路径network_datalink.share.xxxx
		int fd;//共享文件描述符
		char buf[BUFSIZE];//生成的数据
		while(1)
		{
			getSharedFilePath(index,sharedFilePath);
			fd=open(sharedFilePath,O_CREAT | O_WRONLY,0666);
			if(fd<0)//文件创建/打开失败
			{
				printf("Shared file %s open fail!\n",sharedFilePath);
				continue;
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
			index++;
			sleep(1);
			if(index>5)
				break;
		}
		while(1)
			sleep(1);
		exit(0);
	}
	else if(nt_pid>0)//父进程
	{
		//printf("father_layer:%d\n",getpid());
		while((dl_pid=fork())<0)
			sleep(1);
		if(dl_pid==0)//数据链路层进程
		{
			char new_name[20] = "sender1_dtlink";
			prctl(PR_SET_NAME, new_name);
			
			Frame s;
			Packet buffer;
			int index=1;
			char sharedFilePath[PATHLENGTH];//共享文件名/路径network_datalink.share.xxxx
			
			while(1)
			{
				sleep(1);//等待网络层先写数据
				getSharedFilePath(index,sharedFilePath);
				from_network_layer(&buffer,sharedFilePath);
				int i=0;
				for(i=0;i<BUFSIZE;i++)
					(s.info.data)[i]=(buffer.data)[i];
				to_physical_layer(&s);
				index++;
				if(index>5)
					break;
			}
			while(1)
				sleep(1);
			exit(0);			
		}
		else if(dl_pid>0)//父进程
		{
			//printf("father_layer:%d\n",getpid());
			while((ps_pid=fork())<0)
				sleep(1);
			if(ps_pid==0)//物理层进程
			{
				char new_name[20] = "sender1_physic";
				prctl(PR_SET_NAME, new_name);
				Frame s;
				Packet buffer;
				
				int index=1;
				int fd;
				char testPath[PATHLENGTH];
				while(1)
				{
					physical_layer_from_datalink(&s);
					getTestPath(index,testPath);
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
					index++;
					if(index>5)
						break;
				}
				while(1)
					sleep(1);
				exit(0);
			}
			else if(ps_pid>0)//父进程
			{
				printf("father_layer:%d\n",getpid());
				exit(0);
			}
				
		}
	}
}

