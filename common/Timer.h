#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

/* 定时器时间间隔定义 */
#define INTERVAL 1 //时间暂时定为间隔1s

/* 帧编号 */
typedef unsigned int seq_nr;

/* 定时器节点 */
typedef struct TimerNode
{
    unsigned int clk;   //时间
    seq_nr seq;         //序号
    struct TimerNode* nxt;     //指针
}TimerNode,*TimerNodeLink;

/* 定时器 */
typedef struct Timer
{
    TimerNodeLink head; //头指针
    TimerNodeLink tail; //尾指针
    unsigned int sumclk;
}Timer;

/* 创建一个定时节点 */
TimerNodeLink newTimer();

/* 创建定时器 */
struct Timer timer;

