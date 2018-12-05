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
	
    if (read(fd,buffer->data, BUFSIZE) < 0)
	{
		printf("Read file %s fail!\n",sharedFilePath);
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
	if(mkfifo(FIFO_TO_PHYSICAL,0666)==-1 && errno!=EEXIST)//如果不是已存在文件而创建失败，则异常退出
	{
		printf("Create fifo %s fail!",FIFO_TO_PHYSICAL);
		return FALSE;
	}
	
	int fd;
	fd=open(FIFO_TO_PHYSICAL,O_WRONLY,0666);
	if(fd<0)//打开失败
	{
		printf("Open file %s fail!\n",FIFO_TO_PHYSICAL);
		return FALSE;
	}
	if((write(fd,s,BUFSIZE))<0)
	{
		printf("Write pipe %s fail!\n",FIFO_TO_PHYSICAL);
		return FALSE;
	}
	close(fd);
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
	if(mkfifo(FIFO_TO_DATALINK,0666)==-1 && errno!=EEXIST)//如果不是已存在文件而创建失败，则异常退出
	{
		printf("Create fifo %s fail!",FIFO_TO_DATALINK);
		return FALSE;
	}
	
	int fd;
	fd=open(FIFO_TO_DATALINK,O_RDONLY,0666);
	if(fd<0)//打开失败
	{
		printf("Open file %s fail!\n",FIFO_TO_DATALINK);
		return FALSE;
	}
	if((read(fd,r,BUFSIZE))<0)
	{
		printf("Read pipe %s fail!\n",FIFO_TO_DATALINK);
		return FALSE;
	}
	close(fd);
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
	if(mkfifo(FIFO_TO_NETWORK,0666)==-1 && errno!=EEXIST)//如果不是已存在文件而创建失败，则异常退出
	{
		printf("Create fifo %s fail!",FIFO_TO_NETWORK);
		return FALSE;
	}
	
	int fd;
	fd=open(FIFO_TO_NETWORK,O_WRONLY,0666);
	if(fd<0)//打开失败
	{
		printf("Open file %s fail!\n",FIFO_TO_NETWORK);
		return FALSE;
	}
	if((write(fd,buffer,BUFSIZE))<0)
	{
		printf("Write pipe %s fail!\n",FIFO_TO_NETWORK);
		return FALSE;
	}
	close(fd);
	return OK;
}


/**********************
* 函数名称：wait_for_event
* 功    能：等待事件的发生，并用event记录发生的事件类型
* 参    数：event--发生的事件
* 返    回：
* 说    明：
	(1)供datalink进程使用
	(2)处理各种信号，并将信号对应的事件通过event传递
***********************/
void wait_for_event(event_type* event)
{
	
}

/**********************
* 函数名称：start_timer
* 功    能：发送方发送数据后，启动帧k的计时器，如果超时就timeout
* 参    数：帧序号
* 返    回：
* 说    明：SIGALARM信号，精度在ms级，不使用alarm函数
***********************/
void start_timer(seq_nr k)
{
	
}

/**********************
* 函数名称：stop_timer
* 功    能：发送方收到接收方发来的确认帧后，关闭帧k的计时器
* 参    数：帧序号
* 返    回：
* 说    明：
***********************/
void stop_timer(seq_nr k)
{
	
}


/**********************
* 函数名称：start_ack_timer
* 功    能：
* 参    数：
* 返    回：
* 说    明：
***********************/
void start_ack_timer()
{
	
}

/**********************
* 函数名称：stop_ack_timer
* 功    能：
* 参    数：
* 返    回：
* 说    明：
***********************/
void stop_ack_timer()
{
	
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
	strcpy(path,"network_datalink.share.");
	sprintf(tmp,"%d",k);
	strcat(path,tmp);
}