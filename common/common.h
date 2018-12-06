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
#include <signal.h>
#include <sys/prctl.h>//修改进程名

#define MAX_PKT 1024 //数据
#define FALSE 0
#define ERROR -1
#define OK 1

#define BUFSIZE 1024 //每次发送1024字节
#define DATASIZE 1024*1024 //共发送1G字节的数据
#define PATHLENGTH 40	//共享文件名长度
#define FRAMESIZE 1024+12	//数据帧的长度

/*管道文件名宏定义*/
#define FIFO_TO_PHYSICAL "fifo_to_physical.file" //datalink to physical
#define FIFO_TO_DATALINK "fifo_to_datalink.file" //
#define FIFO_TO_NETWORK "fifo_to_network.file"

/*各层之间事件通知信号宏定义*/
#define SIG_CHSUM_ERR	35
#define SIG_FRAME_ARRIVAL 36
#define SIG_NETWORK_LAYER_READY 37
#define SIG_ENABLE_NETWORK_LAYER 38
#define SIG_DISABEL_NETWORK_LAYER 39

typedef int Status;
typedef enum {frame_arrival=1,chsum_err,timeout,ack_timeout,network_layer_ready} event_type;
typedef enum {data,ack,nak} frame_kind;
typedef unsigned int seq_nr;	//帧编号
//#define MAX_SEQ 1 //序号  //留给各个协议实现的文件里去定义

//状态枚举量 false-0 true-1
typedef enum
{
    false,
    true
}Boolen;

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

/* typedef struct TimerNode
{
    unsigned int clk;   //时间
    seq_nr seq;         //序号
    TimerNode* nxt;     //指针
}TimerNode,*TimerNodeLink;

typedef struct
{
    TimerNodeLink head=NULL; //头指针
    TimerNodeLink tail=NULL; //尾指针
    unsigned int sumclk;
}Timer; */


/*全局变量*/
int sig_num;
// Timer timer;//定时器


//数据链路层从网络层（xxx_network进程）获取数据包，存入buffer中
Status from_network_layer(Packet* buffer,char sharedFilePath[]);

//数据链路层将数据帧s传递到物理层（xxx_physical进程）
Status to_physical_layer(Frame* s);


//数据链路层从物理层（xxx_physical进程)获得数据帧，存入r中
Status from_physical_layer(Frame* r);

//数据链路层将数据包buffer传递到网络层(xxx_network进程
Status to_network_layer(Packet* buffer);

//物理层从数据链路层获得数据帧，存入r中
Status physical_layer_from_datalink(Frame *r);


//等待事件的发生，并用event记录发生的事件类型
void wait_for_event(event_type* event);


//创建一个定时器
//TimerNodeLink newTimer();

//发送方发送数据后，启动帧k的计时器，如果超时就timeout
//SIGALARM信号，精度在ms级，不使用alarm函数
void start_timer(seq_nr k);

//发送方收到接收方发来的确认帧后，关闭帧k的计时器
void stop_timer(seq_nr k);

void start_ack_timer();

void stop_ack_timer();

void enable_network_layer();

void disable_network_layer();

//根据k值返回对应的文件名network_datalink.share.xxxx
void getSharedFilePath(int k,char path[]);

//为文件描述符fd对应的文件上锁
Status lock_set(int fd, int type) ;
 void generateData(char buf[]);
 void getTestPath(int k,char path[]);