/* protocol6.h */
#include "../common/common.h"



/* -------------网络层+物理层 宏定义、全局变量以及函数------------ */

#define SEND_FILE "send.dat" //需要发送的数据文件
#define RECV_FILE "received.dat" //接收到的数据文件

int count=1;//网络层与数据链路层共享文件序号1-999
int is_End=0;//网络层进程结束标志

/* 网络层捕捉到SIG_ENABLE_NETWORK信号后，向数据链路层传递数据 */
void network_write(int sig);

/* 从SEND_FILE中每次读取perCount字节到buf */
void readSendFile(char buf[],int perCount);

/* ---------------------------------------------------------------- */

/* -----------数据链路层 宏定义、全局变量以及函数------------------ */

#define MAX_SEQ 7	//最大序号
#define NR_BUFS	((MAX_SEQ+1)/2) //窗口数
int no_nak = TRUE;
seq_nr oldest_frame = MAX_SEQ+1;//初始非法

/* ---------------------------------------------------------------- */
#define inc(k) if(k<MAX_SEQ) k=k+1; else k=0; 

/* 判断接收帧b是否落在接收窗口内 */
static int between(seq_nr a,seq_nr b,seq_nr c);

/*  */
static void send_data(frame_kind fk,seq_nr frame_nr,seq_nr frame_expected,Packet buffer[]);






