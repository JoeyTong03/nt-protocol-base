/* protocol6_network.c */
#include "../common/common.h"
#include "protocol6.h"
int main()
{
	mkdir("File",0666); //存放与数据链路层的共享文件(发送)

	/* 修改进程名，便于根据进程名获得pid值，发送信号 */
	char new_name[20] = "network"; 
	prctl(PR_SET_NAME, new_name);
	
	/* 读取数据 */
	Packet buffer;
	FILE *fp=NULL;
	fp=fopen(RECV_FILE,"a");//以追加写的方式打开文件
	if(fp==NULL)
	{
		printf("Data file open fail!\n");
		exit(-1);
	}
	
	/* 捕捉数据链路层的信号，当窗口有空闲时触发网络层的写操作 */
	(void) signal(SIG_ENABLE_NETWORK,network_write);
	
	while(1)
	{
		/* 从数据链路层接收数据*/
		network_layer_from_datalink(&buffer);
		
		/* 检查接收的数据包是否是全尾零数据包--结束标志 */
		if(strcmp(tmp,buffer.data)==0)
			break;
	
		/* 写入文件 */
		fwrite(&buffer,MAX_PKT,1,fp);
	}
	fclose(fp);//结束读
	
	/* 如果写还未结束，则延时等待 */
	while(!isEnd)
		sleep(1);
	
	exit(0);
}