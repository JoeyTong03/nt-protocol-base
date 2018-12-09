/* protocol6_physical_c.c */
#include "protocol6.h"
#include "../common/common.h"

int main(int argc,char **argv)
{
	/* 修改进程名，便于根据进程名获得pid值，发送信号 */
	char new_name[20] = "physical";
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
	
	/*设置非阻塞*/
	int flags2 = fcntl(client_sockfd, F_GETFL, 0);        //获取文件的flags2值。
    fcntl(client_sockfd, F_SETFL, flags2 | O_NONBLOCK);   //设置成非阻塞模式；
		
	/*设置非阻塞*/
	int flags2 = fcntl(client_sockfd, F_GETFL, 0);        //获取文件的flags2值。
    fcntl(client_sockfd, F_SETFL, flags2 | O_NONBLOCK);   //设置成非阻塞模式；
	
	fd_set read_fds;
	fd_set write_fds;
	int Maxfd;
	Maxfd=client_sockfd;
	
	Frame frame_send;
	Frame frame_recv;
	Packet buffer;
	char tmp[MAX_PKT];
	memset(tmp,'\0',MAX_PKT);
	int isEnd_send=0;
	int isEnd_recv=0;
	while(1)
	{
		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
		if(!isEnd_recv)
			FD_SET(client_sockfd,&read_fds);
		if(!isEnd_send)
			FD_SET(client_sockfd,&write_fds);
		
		int ret=select(Maxfd+1,&read_fds,&write_fds,NULL,NULL);
		switch(ret)
		{
			case -1:
				printf("select error!\n");
				break;
			case 0:
				printf("time out!\n");
				break;
			default:
			{
				/* 如果有可读数据 */
				if(FD_ISSET(client_sockfd,&read_fds))
				{
					FD_CLR(client_sockfd,&read_fds);
					if(read(client_sockfd,&frame_recv,FRAMESIZE)<0)
					{
						printf("TCP socket read fail!\n");
						return;
					}		
					physical_layer_to_datalink(&frame_recv);//将数据传递到数据链路层,通过有名管道方式
					if(strcmp(tmp,frame_recv.info.data)==0)//如果接收到全尾零的数据包
						isEnd_recv=1;
				}

				if(FD_ISSET(client_sockfd,&write_fds))
				{
					FD_CLR(client_sockfd,&write_fds);
					
					/* 从数据链路层通过有名管道获取数据帧 */
					physical_layer_from_datalink(&frame_send);
					if(write(client_sockfd,&frame_send,FRAMESIZE)<0)
					{
						printf("TCP socket write fail!\n");
						return;
					}
					
					/* 检查是否是全\0数据包 */
					if(strcmp(tmp,frame_send.info.data)==0)//是全尾零数据包
						isEnd_send=1;
				}
			}
			
		}
		if(isEnd_recv&&isEnd_send)//接收都结束
			break;
	}

	printf("Physical layer closed!\n");
	close(client_sockfd);
	exit(0);
}