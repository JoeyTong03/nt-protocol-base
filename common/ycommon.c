

#include "common.h"

Timer timer;

TimerNodeLink newTimer()
{
    TimerNodeLink tmptimer = TimerNodeLink malloc(sizeof(TimerNode));
    tmptimer->clk = 0;
    tmptimer->nxt = NULL;
    tmptimer->seq = 0;
    return tmptimer;
}

//启动第k帧的定时器
void start_timer(seq_nr k)
{
    if (timer.head == NULL)
    {
        TimerNodeLink tmp = newTimer();
        tmp->seq = k;
        tmp->clk = 1;

        timer.head = tmp;
        timer.tail = tmp;
        timer.sumclk = clk;

        return;
    }

    TimerNodeLink tmp = newTimer();
    tmp->seq = k;
    tmp->clk = timer.sumclk - clk;

    timer.tail->nxt = tmp;
    timer.sumclk + clk;
}

//停止第k帧的定时器
void stop_timer(seq_nr k)
{
    TimerNodeLink tmp = timer.head;
    if (tmp->seq == K)
    {
        if (timer.head->nxt != NULL)
        {
            timer.head = timer.head->nxt;
            free(tmp);
        }
        else
        {
            timer.head=NULL;
            timer.tail=NULL;
            free(tmp);
        }
    }

    int i = 0;
    for (i = 0; tmp->nxt != NULL; tmp = tmp->nxt)
    {
        if (tmp->nxt->seq == k)
        {
            TimerNodeLink tmpn=tmp->nxt;
            tmp->nxt=tmpn->nxt;
            free(tmpn);
        }
    }
}
