
#define MAX_PKT 1024

//状态枚举量 false-0 true-1
typedef enum
{
    false,
    true
}boolen;

//发送序号
typedef unsigned int seq_nr;

//数据包，纯数据
typedef struct{
    unsigned char data[MAX_PKT];
}packet;

//帧类型枚举量
typedef enum{
    data,   //数据包
    ack,    //确认包
    nak     //否定确认包
}frame_kind;

//帧结构
typedef struct{
    frame_kind kind;    //帧类型
    seq_nr  seq;        //发送序号
    seq_nr  ack;        //接收序号
    packet  info;       //数据包
}frame;

//事件类型枚举量
typedef enum{
    frame_arrival,  //帧到达
    cksum_err,      //检验和错
    timeout,        //发送超时
    network_layer_ready, //网络层就绪
    ack_timeout     //确认包超时
}event_type;

typedef struct TimerNode
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
}Timer;