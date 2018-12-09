/* sender1.c */

#include "../common/common.h"
int count = 1;	 //共享文件序号
int datacount = 0; //已发送数据字节数
int isEnd = 0;
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
	char sharedFilePath[PATHLENGTH]; //共享文件名/路径network_datalink.share.xxxx
	int fd;							 //共享文件描述符
	char buf[MAX_PKT];				 //生成的数据
	int perCount = 1024;
	int tmp_isEnd = 0;
	if (datacount <= TOTALDATASIZE) //
	{
		if (TOTALDATASIZE - datacount < MAX_PKT) //最后一个数据报不足1024字节
			perCount = TOTALDATASIZE - datacount;
		readDataFromFile(buf, perCount); //从1G文件中每次读取perCount字节
		datacount += 1024;
	}
	else
	{
		memset(buf, '\0', MAX_PKT); //作为结束标志的全尾零文件标志
		tmp_isEnd = 1;
	}

	getSharedFilePath(count, sharedFilePath);
	fd = open(sharedFilePath, O_CREAT | O_WRONLY, 0666);
	if (fd < 0) //文件创建/打开失败
	{
		printf("Shared file %s open fail!\n", sharedFilePath);
		return;
	}
	/*为共享文件加写锁*/
	if (lock_set(fd, F_WRLCK) == FALSE) //上写锁失败
		exit(-1);
	//continue;

	/*向共享文件中写入要传递的数据*/
	if (write(fd, buf, MAX_PKT) < 0)
	{
		printf("Write share file %s fail!\n", sharedFilePath);
		lock_set(fd, F_UNLCK); //退出前先解锁
		exit(-1);
		//continue;
	}
	lock_set(fd, F_UNLCK); //退出前先解锁
	close(fd);
	enable_datalink_read(); //通知数据链路层读数据
	count++;
	if (count > 999)
		count = 1; //轮询使用
	if (tmp_isEnd)
		isEnd = tmp_isEnd;
}

/**********************
 * 函数名称：datalink_read
 * 功    能：数据链路层接收到_DATALINK_RD信号后的处理函数，
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
	event_type event;
	char sharedFilePath[PATHLENGTH]; //共享文件名/路径network_datalink.share.xxxx

	getSharedFilePath(count, sharedFilePath);

	/* 从网络层获取数据，通过共享文件+信号方式 */
	from_network_layer(&buffer, sharedFilePath);

	/* 检查是否是全\0数据包 */
	int tmp_isEnd = 0;
	char tmp[MAX_PKT];
	memset(tmp, '\0', MAX_PKT);
	if (strcmp(tmp, buffer.data) == 0) //是全尾零数据包
		tmp_isEnd = 1;

	/* 将从网络层获取的数据包“装入”数据帧 */
	int i = 0;
	for (i = 0; i < MAX_PKT; i++)
		(s.info.data)[i] = (buffer.data)[i];

	/* 将数据帧传递给物理层 */
	to_physical_layer(&s);

	enable_network_write(); //通知网络层写数据
	count++;				//该函数只在数据链路层中被调用，
	if (count > 999)		//轮训读数据
		count = 1;
	if (tmp_isEnd)
		isEnd = 1;

	wait_for_event(&event); //等到事件

}

//网络层所做的事情
void network_layer()
{
	/* 修改进程名，便于根据进程名获得pid值，发送信号 */
	char new_name[20] = "sender3_network";
	prctl(PR_SET_NAME, new_name);

	/* 
			等待数据链路层读完共享文件数据后发“写”信号，
			随机生成1024字节数据写入共享文件
			实现网络层到数据链路层的数据传递
			*/
	mkdir("File", 0666);
	(void)signal(SIG_WR, network_write);

	while (1)
	{
		if (isEnd)
			break;
		sleep(1);
	}
	exit(0);
}

//数据链路层所做的事情
void datalink_layer()
{
	/* 修改进程名，便于根据进程名获得pid值，发送信号 */
	char new_name[20] = "sender1_dtlink";
	prctl(PR_SET_NAME, new_name);

	/* 
				等待网络层写完共享文件数据后发“读”信号，
				接收网络层数据并封装成数据帧发送给物理层
				*/
	(void)signal(SIG_RD, datalink_read);

	while (1)
	{
		if (isEnd)
			break;
		sleep(1);
	}

	exit(0);
}

//物理层
void physical_layer(int port)
{
	/* 修改进程名，便于根据进程名获得pid值，发送信号 */
	char new_name[20] = "sender1_physic";
	prctl(PR_SET_NAME, new_name);

	/* 使用TCP套接字通信模拟物理线路通信 */
	int server_sockfd;				//服务器端套接字
	int client_sockfd;				//客户端套接字
	struct sockaddr_in my_addr;		//服务器套接字地址结构体
	struct sockaddr_in remote_addr; //客户端套接字地址结构体
	int sin_size;
	memset(&my_addr, 0, sizeof(my_addr)); //服务器套接字地址结构体初始化--清零
	my_addr.sin_family = AF_INET;		  //因特网通信协议
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	/*创建服务器端套接字*/
	if ((server_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		exit(-1);
	}

	/*设置SO(_REUSEADDR*/
	int enable = 1;
	if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");

	/*将套接字绑定到服务器的网络地址上*/
	if (bind(server_sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0)
	{
		perror("bind error");
		exit(-1);
	}

	/*监听连接请求，监听队列长度为5*/
	if (listen(server_sockfd, 5) < 0)
	{
		perror("listen error");
		exit(-1);
	}

	sin_size = sizeof(struct sockaddr_in);
	/*等待客户端连接请求到达*/
	if ((client_sockfd = accept(server_sockfd, (struct sockaddr *)&remote_addr, &sin_size)) < 0)
	{
		perror("accept error");
		exit(-1);
	}

	enable_network_write(); //物理层准备好后通知应用层写数据

	Frame s;
	Packet buffer;
	int count = 1;
	char tmp[MAX_PKT];
	memset(tmp, '\0', MAX_PKT);
	/*接收客户端的数据并将其发送给客户端，resv返回收到的字节数，send返回发送的字节数*/
	while (1)
	{
		/* 从数据链路层通过有名管道获取数据帧 */
		physical_layer_from_datalink(&s);
		if (write(client_sockfd, &s, FRAMESIZE) < 0)
		{
			printf("Write socket fail!\n");
			continue;
		}
		count++;

		/* 检查是否是全\0数据包 */
		if (strcmp(tmp, s.info.data) == 0) //是全尾零数据包
			break;
	}
	printf("Send end!\n");
	close(client_sockfd);
	exit(0);
}

int main(int argc, char **argv)
{
	pid_t nt_pid; //网络层进程的pid号
	pid_t dl_pid; //数据链路层进程的pid号
	pid_t ps_pid; //物理层进程的pid号
	while ((nt_pid = fork()) < 0)
		sleep(1);

	if (nt_pid == 0) //网络层进程
	{
		network_layer();
	}
	else if (nt_pid > 0) //父进程fork子进程--数据链路层
	{
		while ((dl_pid = fork()) < 0)
			sleep(1);
		if (dl_pid == 0) //数据链路层进程
		{
			datalink_layer();
		}
		else if (dl_pid > 0) //父进程fork子进程--物理层
		{
			while ((ps_pid = fork()) < 0)
				sleep(1);
			if (ps_pid == 0) //物理层进程
			{
				physical_layer(atoi(argv[1]));
			}
			else if (ps_pid > 0) //父进程
				exit(0);
		}
	}
}
