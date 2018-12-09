/*common.c*/
#include "common.h"
/**********************
* 函数名称：from_network_layer
* 功    能：从网络层（xxx_network进程）获取数据包，存入buffer中
* 参    数：buffer--获取到的数据包;sharedFilePath---共享文件路径
* 返    回：Status: 0--文件打开失败
* 说    明：供xxx_datalink进程使用，xxx与上一致；使用文件锁机制
***********************/
Status from_network_layer(Packet* buffer,char sharedFilePath[])
{
	int fd;
	fd=open(sharedFilePath,O_RDONLY,0666);//打开共享文件
	if(fd<0)//打开失败
	{
		printf("Open file %s fail!\n",sharedFilePath);
		return FALSE;
	}
	
	/* 给文件上读入锁 */  
	lock_set(fd, F_RDLCK); 

    if (read(fd,buffer->data, MAX_PKT) < 0)
	{
		printf("Read file %s fail!\n",sharedFilePath);
		lock_set(fd, F_UNLCK); //退出前先解锁
		return FALSE;
	}   

	/* 给文件解锁 */  
	lock_set(fd, F_UNLCK);    
	close(fd);  
	return OK;
}

/**********************
* 函数名称：to_physical_layer
* 功    能：将数据帧s传递到物理层（xxx_physical进程）
* 参    数：s--需要发送的帧
* 返    回：Status--是否成功
* 说    明：
	(1)供xxx_datalink进程使用,xxx与上一致
	(2)使用有名管道实现进程间数据传递
***********************/
Status to_physical_layer(Frame* s)
{
	if(mkfifo(FIFO_DL_TO_PS,0666)==-1 && errno!=EEXIST)//如果不是已存在文件而创建失败，则异常退出
	{
		printf("Create fifo %s fail!",FIFO_DL_TO_PS);
		return FALSE;
	}
	
	int fd;
	fd=open(FIFO_DL_TO_PS,O_WRONLY,0666);
	if(fd<0)//打开失败
	{
		printf("Open file %s fail!\n",FIFO_DL_TO_PS);
		return FALSE;
	}
	if((write(fd,s,FRAMESIZE))<0)
	{
		printf("Write pipe %s fail!\n",FIFO_DL_TO_PS);
		return FALSE;
	}
	close(fd);
	return OK;
}

/**********************
* 函数名称：physical_layer_from_datalink
* 功    能：物理层接收数据链路层的数据帧
* 参    数：s--接收的帧
* 返    回：Status--是否成功
* 说    明：
	(1)供xxx_physical进程使用,xxx与上一致
	(2)使用有名管道实现进程间数据传递
***********************/
Status physical_layer_from_datalink(Frame *s)
{
	if(mkfifo(FIFO_DL_TO_PS,0666)==-1 && errno!=EEXIST)//如果不是已存在文件而创建失败，则异常退出
	{
		printf("Create fifo %s fail!",FIFO_DL_TO_PS);
		return FALSE;
	}
	
	int fd;
	fd=open(FIFO_DL_TO_PS,O_RDONLY,0666);
	if(fd<0)//打开失败
	{
		printf("Open file %s fail!\n",FIFO_DL_TO_PS);
		return FALSE;
	}
	if((read(fd,s,FRAMESIZE))<0)
	{
		printf("Write pipe %s fail!\n",FIFO_DL_TO_PS);
		return FALSE;
	}
	close(fd);
	unlink(FIFO_DL_TO_PS);//删除管道
	return OK;
}

/**********************
* 函数名称：from_physical_layer
* 功    能：从物理层（xxx_physical进程)获得数据帧，存入r中
* 参    数：r--获取到的帧
* 返    回：Status--是否成功
* 说    明：供xxx_datalink进程使用,xxx与上一致
***********************/
Status from_physical_layer(Frame* r)
{
	if(mkfifo(FIFO_PS_TO_DL,0666)==-1 && errno!=EEXIST)//如果不是已存在文件而创建失败，则异常退出
	{
		printf("Create fifo %s fail!",FIFO_PS_TO_DL);
		return FALSE;
	}
	
	int fd;
	fd=open(FIFO_PS_TO_DL,O_RDONLY,0666);
	if(fd<0)//打开失败
	{
		printf("Open file %s fail!\n",FIFO_PS_TO_DL);
		return FALSE;
	}
	if((read(fd,r,FRAMESIZE))<0)
	{
		printf("Read pipe %s fail!\n",FIFO_PS_TO_DL);
		return FALSE;
	}
	close(fd);
	unlink(FIFO_PS_TO_DL);//删除管道
	return OK;
}

/**********************
* 函数名称：to_network_layer
* 功    能：将数据包buffer传递到网络层(xxx_network进程
* 参    数：buffer--需要发送到网络层的数据包
* 返    回：Status--是否成功
* 说    明：供xxx_datalink进程使用,xxx与上一致
***********************/
Status to_network_layer(Packet* buffer)
{
	if(mkfifo(FIFO_DL_TO_NT,0666)==-1 && errno!=EEXIST)//如果不是已存在文件而创建失败，则异常退出
	{
		printf("Create fifo %s fail!",FIFO_DL_TO_NT);
		return FALSE;
	}
	
	int fd;
	fd=open(FIFO_DL_TO_NT,O_WRONLY,0666);
	if(fd<0)//打开失败
	{
		printf("Open file %s fail!\n",FIFO_DL_TO_NT);
		return FALSE;
	}
	if((write(fd,buffer,MAX_PKT))<0)
	{
		printf("Write pipe %s fail!\n",FIFO_DL_TO_NT);
		return FALSE;
	}
	close(fd);
	return OK;
}

/**********************
* 函数名称：network_layer_from_datalink
* 功    能：网络层从数据链路层中获取数据包，存入buffer中;
			并将buffer的内容写入network_datalink.share.xxxx，与发送方网络层一致，便于比较
* 参    数：buffer--获取到的数据包
* 返    回：Status--是否成功
* 说    明：供xxx_network进程使用,xxx与上一致
***********************/
Status network_layer_from_datalink(Packet* buffer)
{
	if(mkfifo(FIFO_DL_TO_NT,0666)==-1 && errno!=EEXIST)//如果不是已存在文件而创建失败，则异常退出
	{
		printf("Create fifo %s fail!",FIFO_DL_TO_NT);
		return FALSE;
	}
	
	int fd;
	fd=open(FIFO_DL_TO_NT,O_RDONLY,0666);
	if(fd<0)//打开失败
	{
		printf("Open file %s fail!\n",FIFO_DL_TO_NT);
		return FALSE;
	}
	if((read(fd,buffer,MAX_PKT))<0)
	{
		printf("Read pipe %s fail!\n",FIFO_DL_TO_NT);
		return FALSE;
	}
	close(fd);
	unlink(FIFO_DL_TO_NT);
	return OK;
}

/**********************
* 函数名称：physical_layer_to_datalink
* 功    能：物理层将数据帧发送给数据链路层
* 参    数：buffer--获取到的数据包
* 返    回：Status--是否成功
* 说    明：供xxx_network进程使用,xxx与上一致
***********************/
Status physical_layer_to_datalink(Frame *r)
{
	if(mkfifo(FIFO_PS_TO_DL,0666)==-1 && errno!=EEXIST)//如果不是已存在文件而创建失败，则异常退出
	{
		printf("Create fifo %s fail!",FIFO_PS_TO_DL);
		return FALSE;
	}
	
	int fd;
	fd=open(FIFO_PS_TO_DL,O_WRONLY,0666);
	if(fd<0)//打开失败
	{
		printf("Open file %s fail!\n",FIFO_PS_TO_DL);
		return FALSE;
	}
	if((write(fd,r,FRAMESIZE))<0)
	{
		printf("Write pipe %s fail!\n",FIFO_PS_TO_DL);
		return FALSE;
	}
	close(fd);
	return OK;
}

/**********************
* 函数名称：wait_for_event
* 功    能：等待信号变化
* 参    数：event--发生的事件
* 返    回：
* 说    明：
	(1)供datalink进程使用
	(2)循环等待信号发生变化，即连续遇到两个相同的信号时会忽略第二个信号！！BE CARE!
***********************/
void wait_for_event(event_type* event)
{
	while(sig_num=0)
		sleep(1);
	
	if(	sig_num==SIG_CHSUM_ERR | sig_num==SIG_FRAME_ARRIVAL |
		sig_num==SIG_NETWORK_LAYER_READY |sig_num==SIG_ENABLE_NETWORK_LAYER |
		sig_num==SIG_DISABEL_NETWORK_LAYER | sig_num==SIGALRM
		)
		*event=sig_num;
	else	
		*event=0;
	sig_num=0;//卸磨杀驴
}


/**********************
* 函数名称：sig_func
* 功    能：信号处理函数，根据收到的信号为全局变量sig_num赋值
* 参    数：sig--收到的信号
* 返    回：
* 说    明：供sig_catch函数使用
***********************/
void sig_func(int sig)  
{  
	sig_num=sig;
}  

/**********************
* 函数名称：sig_catch
* 功    能：捕捉定义的信号，处理函数为sig_func
* 参    数：
* 返    回：
* 说    明：进程开始时需要调用该函数，通过全局变量sig_num的值“传递”信号
***********************/
void sig_catch()
{
	(void) signal(SIG_CHSUM_ERR,sig_func); 
	(void) signal(SIG_FRAME_ARRIVAL,sig_func);
	(void) signal(SIG_NETWORK_LAYER_READY,sig_func);
	(void) signal(SIG_ENABLE_NETWORK_LAYER,sig_func);
	(void) signal(SIG_DISABEL_NETWORK_LAYER,sig_func);
	(void) signal(SIGALRM,sig_func);
}


/**********************
* 函数名称：enable_network_layer
* 功    能：
* 参    数：
* 返    回：
* 说    明：
***********************/
void enable_network_layer()
{
}

/**********************
* 函数名称：disable_network_layer
* 功    能：
* 参    数：
* 返    回：
* 说    明：
***********************/
void disable_network_layer()
{
}

/**********************
* 函数名称：enable_network_write
* 功    能：通知网络层写数据
* 参    数：
* 返    回：
* 说    明：
***********************/
void enable_network_write()
{
	//network_layer_status=1;
	//获取network层进程的pid号
    int pid;
    FILE *fp = popen("ps -e | grep \'network' | awk \'{print $1}\'","r");
    char buffer[10] = {0};
    fgets(buffer, 10, fp);
    pid=atoi(buffer);
	kill(pid,SIG_WR);
}
/**********************
* 函数名称：enable_datalink_read
* 功    能：通知数据链路层读数据
* 参    数：
* 返    回：
* 说    明：
***********************/
void enable_datalink_read()
{
	//network_layer_status=1;
	//获取network层进程的pid号
    int pid;
    FILE *fp = popen("ps -e | grep \'dtlink' | awk \'{print $1}\'","r");
    char buffer[10] = {0};
    fgets(buffer, 10, fp);
    pid=atoi(buffer);
	kill(pid,SIG_RD);
}

/**********************
* 函数名称：enable_physical_write
* 功    能：通知物理层写数据
* 参    数：
* 返    回：
* 说    明：
***********************/
void enable_physical_write()
{
	//network_layer_status=1;
	//获取network层进程的pid号
    int pid;
    FILE *fp = popen("ps -e | grep \'physic' | awk \'{print $1}\'","r");
    char buffer[10] = {0};
    fgets(buffer, 10, fp);
    pid=atoi(buffer);
	kill(pid,SIG_WR);
}
/**********************
* 函数名称：lock_set
* 功    能：为文件描述符fd对应的文件上锁
* 参    数：fd--文件描述符，type--锁的类型
* 返    回：
* 说    明：锁的类型
	(1)F_RDLCK:读锁
	(2)F_WRLCK:写锁
	(3)F_UNLCK:解锁
***********************/
Status lock_set(int fd, int type)  
{  
    struct flock old_lock, lock;  
    lock.l_whence = SEEK_SET;  
    lock.l_start = 0;  
    lock.l_len = 0;  
    lock.l_type = type;  
    lock.l_pid = -1;  
      
    // 判断文件是否可以上锁   
    fcntl(fd, F_GETLK, &lock);  
      
    if (lock.l_type != F_UNLCK)  
    {  
        //判断文件不能上锁的原因 
        if (lock.l_type == F_RDLCK) 
            printf("Read lock already set by %d\n", lock.l_pid);  
        else if (lock.l_type == F_WRLCK)   
            printf("Write lock already set by %d\n", lock.l_pid);  
    }  
      
    lock.l_type = type; 
	printf("The type of lock is %d.\n",type);
    //根据不同的type值进行阻塞式上锁或解锁 
    if ((fcntl(fd, F_SETLKW, &lock)) < 0)  
    {  
        printf("Lock failed:type = %d\n", lock.l_type);  
        return FALSE;  
    }  
          
    switch(lock.l_type)  
    {  
        case F_RDLCK:  
            printf("Read lock set by %d\n", getpid());  
            break;  
  
        case F_WRLCK:  
            printf("Write lock set by %d\n", getpid());  
            break;  
  
        case F_UNLCK:  
            printf("Release lock by %d\n", getpid());  
            break; 
        default:  
            break;  
    }
    return OK;  
}  


/**********************
* 函数名称：getSharedFilePath
* 功    能：根据k值获得对应的共享文件路径，格式为network_datalink.share.xxxx
* 参    数：k--第k个共享文件，path--拼接生成的路径名
* 返    回：
* 说    明：0001-0999
***********************/
void getSharedFilePath(int k,char path[])
{
	char tmp[10];
	memset(path,0,PATHLENGTH);
	memset(tmp,0,10);
	strcpy(path,"File/network_datalink.share.");
	sprintf(tmp,"%04d",k);
	strcat(path,tmp);
}

 /**********************
* 函数名称：readDataFromFile
* 功    能：发送方从1G文件中每次读取perCount字节
* 参    数：buf--读取的数据 perCount--此次读取的数据
* 返    回：
* 说    明：不足1024字节，用\0填充
***********************/
void readDataFromFile(char buf[],int perCount)
{
	int fd;
	fd=open(DATAFILE,O_RDONLY,0666);
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