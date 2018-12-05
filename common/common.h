/*common.h*/
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/file.h>//flock,lockf
#include <errno.h>

#define MAX_PKT 1024
#define FALSE 0
#define ERROR -1
#define OK 1

#define BUFSIZE 1024 //每次发送1024字节

//有名管道从数据链路层向物理层传递数据，需要的fifo文件
#define FIFO_TO_PHYSICAL "fifo_to_physical.file" 
//使用有名管道从物理层向数据链路层传递数据，需要的fifo文件
#define FIFO_TO_DATALINK "fifo_to_datalink.file"
//使用有名管道从数据链路层向网络层，需要的fifo文件
#define FIFO_TO_NETWORK "fifo_to_network.file"

typedef int Status;
typedef enum {frame_arrival,cksum_err,timeout} event_type;
typedef enum {data,ack,nak} frame_kind;
typedef unsigned int seq_nr;	//帧编号
//#define MAX_SEQ 1 //序号  //留给各个协议实现的文件里去定义
typedef struct
{
	unsigned char data[MAX_PKT];
}Packet;

typedef struct
{
	frame_kind kind;	
	seq_nr seq;
	seq_nr ack;
	Packet info;
}Frame;


//从网络层（xxx_network进程）获取数据包，存入buffer中
Status from_network_layer(Packet* buffer,char sharedFilePath[]);

//将数据帧s传递到物理层（xxx_physical进程）
Status to_physical_layer(Frame* s);


//从物理层（xxx_physical进程)获得数据帧，存入r中
Status from_physical_layer(Frame* r);

//将数据包buffer传递到网络层(xxx_network进程
Status to_network_layer(Packet* buffer);

//等待事件的发生，并用event记录发生的事件类型
void wait_for_event(event_type* event);

//发送方发送数据后，启动帧k的计时器，如果超时就timeout
//SIGALARM信号，精度在ms级，不使用alarm函数
void start_timer(seq_nr k);

//发送方收到接收方发来的确认帧后，关闭帧k的计时器
void stop_timer(seq_nr k);

void start_ack_timer();

void stop_ack_timer();

void enable_network_layer();

void disable_network_layer();
