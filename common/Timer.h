#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

/* ��ʱ��ʱ�������� */
#define INTERVAL 1 //ʱ����ʱ��Ϊ���1s

/* ֡��� */
typedef unsigned int seq_nr;

/* ��ʱ���ڵ� */
typedef struct TimerNode
{
    unsigned int clk;   //ʱ��
    seq_nr seq;         //���
    struct TimerNode* nxt;     //ָ��
}TimerNode,*TimerNodeLink;

/* ��ʱ�� */
typedef struct Timer
{
    TimerNodeLink head; //ͷָ��
    TimerNodeLink tail; //βָ��
    unsigned int sumclk;
}Timer;

/* ����һ����ʱ�ڵ� */
TimerNodeLink newTimer();

/* ������ʱ�� */
struct Timer timer;

