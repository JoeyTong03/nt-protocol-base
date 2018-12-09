/* receiver.c */
#include "../common/common.h"
int count=1;
int isEnd=0;
int client_sockfd;
void physical_write(int sig)
{
	Frame r;
	if(read(client_sockfd,&r,FRAMESIZE)<0)
	{
		printf("TCP socket read fail!\n");
		return;
	}		
	physical_layer_to_datalink(&r);//将数据传递到数据链路层,通过有名管道方式
	
	/* 检查接收到的是否是全零包 */
	char tmp[MAX_PKT];
	memset(tmp,'\0',MAX_PKT);
	if(strcmp(tmp,r.info.data)==0)
		isEnd=1;
	else
	{
		printf("[Physical layer send]:%d\n",count);
		count++;
	}
	
	
}
int main(int argc,char *argv[])
{
	pid_t nt_pid;//网络层进程的pid号
	pid_t dl_pid;//数据链路层进程的pid号
	pid_t ps_pid;//物理层进程的pid号
	while((nt_pid=fork())<0)
		sleep(1);
	
	if(nt_pid==0)//网络层进程
	{
		mkdir("File",0666);
		/* 修改进程名，便于根据进程名获得pid值，发送信号 */
		char new_name[20] = "recv1_network"; 
		prctl(PR_SET_NAME, new_name);
		
		Packet buffer;
		int count=1;
		char fileName[PATHLENGTH];
		char tmp[MAX_PKT];
		memset(tmp,'\0',MAX_PKT);
		FILE *fp=NULL;
		fp=fopen(DATAFILE,"a");//以追加写的方式打开文件
		if(fp==NULL)
		{
			printf("Data file open fail!\n");
			exit(-1);
		}
		while(1)
		{
			enable_physical_write();//通知物理层写数据
			//getSharedFilePath(count,fileName);
			
			/* 从数据链路层接收数据*/
			network_layer_from_datalink(&buffer);
			
			if(strcmp(tmp,buffer.data)==0)
				break;
		
			/* 写入文件 */
			fwrite(&buffer,MAX_PKT,1,fp);
			printf("[Network layer recv]:%d\n",count);
			count++;
		}
		
		/* 处理倒数第二份数据包的尾零 */
		
		fclose(fp);
		exit(0);
			
	}
	else if(nt_pid>0)//父进程fork子进程--数据链路层
	{
		while((dl_pid=fork())<0)
			sleep(1);
		if(dl_pid==0)//数据链路层进程
		{
			/* 修改进程名，便于根据进程名获得pid值，发送信号 */
			char new_name[20] = "recv1_dtlink";
			prctl(PR_SET_NAME, new_name);
			Frame r;
			Packet buffer;
			char tmp[MAX_PKT];
			memset(tmp,'\0',MAX_PKT);
			while(1)
			{
				/* 从物理层接收数据帧 */
				from_physical_layer(&r);
				printf("[Datalink layer recv]:%d\n",count);
								
				/* 将数据帧中的数据包剥离出来，交给buffer */
				int i=0;
				for(i=0;i<MAX_PKT;i++)
					buffer.data[i]=(r.info.data)[i];
				
				/* 将数据包传递给网络层 */
				to_network_layer(&buffer);
				
				/* 检查是否是全零包 */
				if(strcmp(tmp,r.info.data)==0)
					break;
				
				printf("[Datalink layer send]:%d\n",count);			
			}
			exit(0);
		}
		else if(dl_pid>0)//父进程fork子进程--物理层
		{
			
			while((ps_pid=fork())<0)
				sleep(1);
			if(ps_pid==0)//物理层进程
			{
				/* 修改进程名，便于根据进程名获得pid值，发送信号 */
				char new_name[20] = "recv1_physic";
				prctl(PR_SET_NAME, new_name);
				
				struct sockaddr_in remote_addr;//客户端网络地址结构体
				memset(&remote_addr,0,sizeof(remote_addr));
				remote_addr.sin_family=AF_INET;//通信协议为因特网
				remote_addr.sin_port=htons(atoi(argv[2]));//服务器端口号
				remote_addr.sin_addr.s_addr=inet_addr(argv[1]);//服务器IP地址
				/*创建客户端套接字*/
				if((client_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
				{
					perror("socket error");
					return 1;
				}

				/*将套接字绑定到客户端网络地址上*/
				if(connect(client_sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)
				{
					perror("connect error");
					return 1;
				}
				
				/* 捕捉网络层发出的“写”信号后再读取数据，并向数据链路层传递 */
				(void) signal(SIG_WR,physical_write);

				while(1)
				{
					if(isEnd)
						break;
					sleep(1);
				}
				printf("receive end!\n");
				close(client_sockfd);
				exit(0);
			}
			else if(ps_pid>0)//父进程
				exit(0);	
		}
	}
}