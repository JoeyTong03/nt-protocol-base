/*Timer.c*/

#include "common.h"
#include "Timer.h"

/**********************
 * 函数名称：start_timer
 * 功    能：发送方发送数据后，启动帧k的计时器，如果超时就timeout
 * 参    数：帧序号
 * 返    回：
 * 说    明：SIGALARM信号，精度在ms级，不使用alarm函数
***********************/
void start_timer(seq_nr k)
{
	if (timer.head == NULL)
	{
		TimerNodeLink tmp = NULL;
		tmp = (TimerNodeLink)malloc(sizeof(struct TimerNode));
		tmp->clk = INTERVAL;
		tmp->nxt = NULL;
		tmp->seq = k;

		timer.head = tmp;
		timer.tail = tmp;
		timer.sumclk = INTERVAL;

		return;
	}

	TimerNodeLink tmp = NULL;
	tmp = (TimerNodeLink)malloc(sizeof(struct TimerNode));
	tmp->clk = timer.sumclk - INTERVAL;
	tmp->seq = k;
	timer.tail->nxt = tmp;
	timer.tail = tmp;
	timer.sumclk + INTERVAL;
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
	TimerNodeLink tmp = timer.head;
	if (tmp->seq == k)
	{
		if (timer.head->nxt != NULL)
		{
			timer.head = timer.head->nxt;
			free(tmp);
		}
		else
		{
			timer.head = NULL;
			timer.tail = NULL;
			free(tmp);
		}
	}

	int i = 0;
	for (i = 0; tmp->nxt != NULL; tmp = tmp->nxt)
	{
		if (tmp->nxt->seq == k)
		{
			TimerNodeLink tmpn = tmp->nxt;
			tmp->nxt = tmpn->nxt;
			free(tmpn);
		}
	}
}

//超时处理
void Process_TIMEOUT(int sig, siginfo_t *info, void *act)
{

}

void CreateSigAndBind(int sig, void (*fun)(int sig, siginfo_t *info, void *myact))
{
    struct sigaction act;     //创建新的信号
    struct sigaction old_act; //记录旧的信号

    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = fun; //设置信号的响应操作

    if (sigaction(sig, &act,&old_act)<0)
    {
        perror("install signal error\n");
        return;
    }
}


/**********************
 * 函数名称：sig_ms
 * 功    能：运行一个时钟周期(1ms)所处理的事情
 * 参    数：
 * 返    回：
 * 说    明：
***********************/
void sig_ms(int signo)
{
	if (timer != NULL)
	{
		//定时器已经开启了

		TimerNodeLink tmp = timer.head;
		tmp->clk--;

		//如果这个节点的时钟到点了
		if (tmp->clk == 0)
		{
			//发送信号 - 超时信号
			signal(SIG_TIMEOUT);			

			//节点 - 如果是尾节点，就NULL了
			timer.head=tmp->nxt;
			free(tmp);
		}
	}
}

/**********************
 * 函数名称：run_timer
 * 功    能：运行计时器ms
 * 参    数：
 * 返    回：
 * 说    明：
***********************/
void run_timer()
{
	timer.head = NULL;
	timer.tail = NULL;

	//设置时间间隔为1ms
	struct itimerval value, ovalue;
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 1000;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 1000;

	//创建信号
    CreateSigAndBind(SIG_TIMEOUT,&Process_TIMEOUT);

	//信号关联
	signal(SIGALRM, sig_ms);

	//开启定时器 - 超时发送SIGALRM信号
	setitimer(ITIMER_REAL, &value, &ovalue);

	while (1)
		;
}