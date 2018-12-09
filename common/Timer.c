/*Timer.c*/

#include "common.h"
#include "Timer.h"

/**********************
 * �������ƣ�start_timer
 * ��    �ܣ����ͷ��������ݺ�����֡k�ļ�ʱ���������ʱ��timeout
 * ��    ����֡���
 * ��    �أ�
 * ˵    ����SIGALARM�źţ�������ms������ʹ��alarm����
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
 * �������ƣ�stop_timer
 * ��    �ܣ����ͷ��յ����շ�������ȷ��֡�󣬹ر�֡k�ļ�ʱ��
 * ��    ����֡���
 * ��    �أ�
 * ˵    ����
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

//��ʱ����
void Process_TIMEOUT(int sig, siginfo_t *info, void *act)
{

}

void CreateSigAndBind(int sig, void (*fun)(int sig, siginfo_t *info, void *myact))
{
    struct sigaction act;     //�����µ��ź�
    struct sigaction old_act; //��¼�ɵ��ź�

    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = fun; //�����źŵ���Ӧ����

    if (sigaction(sig, &act,&old_act)<0)
    {
        perror("install signal error\n");
        return;
    }
}


/**********************
 * �������ƣ�sig_ms
 * ��    �ܣ�����һ��ʱ������(1ms)�����������
 * ��    ����
 * ��    �أ�
 * ˵    ����
***********************/
void sig_ms(int signo)
{
	if (timer != NULL)
	{
		//��ʱ���Ѿ�������

		TimerNodeLink tmp = timer.head;
		tmp->clk--;

		//�������ڵ��ʱ�ӵ�����
		if (tmp->clk == 0)
		{
			//�����ź� - ��ʱ�ź�
			signal(SIG_TIMEOUT);			

			//�ڵ� - �����β�ڵ㣬��NULL��
			timer.head=tmp->nxt;
			free(tmp);
		}
	}
}

/**********************
 * �������ƣ�run_timer
 * ��    �ܣ����м�ʱ��ms
 * ��    ����
 * ��    �أ�
 * ˵    ����
***********************/
void run_timer()
{
	timer.head = NULL;
	timer.tail = NULL;

	//����ʱ����Ϊ1ms
	struct itimerval value, ovalue;
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 1000;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 1000;

	//�����ź�
    CreateSigAndBind(SIG_TIMEOUT,&Process_TIMEOUT);

	//�źŹ���
	signal(SIGALRM, sig_ms);

	//������ʱ�� - ��ʱ����SIGALRM�ź�
	setitimer(ITIMER_REAL, &value, &ovalue);

	while (1)
		;
}