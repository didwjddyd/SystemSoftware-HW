#define _GNU_SOURCE

#include "init.h"
#include "thread.h"
#include "scheduler.h"

#include <time.h>
#include <stdio.h>

void myHandler_SIGALRM(int signum) {
    // if no thread running
    if(pCurrentThread == NULL) {
        pCurrentThread = ReadyQueue.pHead;
        ReadyQueue.pHead = NULL;
        ReadyQueue.pTail = NULL;
        ReadyQueue.queueCount--;

        if(pCurrentThread->status != THREAD_STATUS_ZOMBIE) {
            pCurrentThread->status = THREAD_STATUS_RUN;
        }

        pCurrentThread->cpu_time += TIMESLICE;
        kill(pCurrentThread->pid, SIGCONT);
        //__ContextSwitch(pCurrentThread->pid, pCurrentThread->pid);
    }
    // if ready queue is empty
    else if(ReadyQueue.queueCount == 0) {
        pCurrentThread->cpu_time += TIMESLICE;
        __ContextSwitch(pCurrentThread->pid, pCurrentThread->pid);
    }
    // if ready queue has TCB
    else if(ReadyQueue.queueCount > 0) {
        int curpid = pCurrentThread->pid;

        // push back running thread into ready queue
        if(pCurrentThread->status != THREAD_STATUS_ZOMBIE) {
            pCurrentThread->status = THREAD_STATUS_READY;
        }
        ReadyQueue.pTail->pNext = pCurrentThread;
        pCurrentThread->pPrev = ReadyQueue.pTail;
        ReadyQueue.pTail = ReadyQueue.pTail->pNext;

        // pop ready queue
        pCurrentThread = ReadyQueue.pHead;
        ReadyQueue.pHead = ReadyQueue.pHead->pNext;
        ReadyQueue.pHead->pPrev = NULL;

        // set new thread
        if(pCurrentThread->status != THREAD_STATUS_ZOMBIE) {
            pCurrentThread->status = THREAD_STATUS_RUN;
        }

        pCurrentThread->pNext = NULL;
        pCurrentThread->cpu_time += TIMESLICE;

        int newpid = pCurrentThread->pid;


        // context switch
        __ContextSwitch(curpid, newpid);
    }
}

void __ContextSwitch(int curpid, int newpid) {
    kill(curpid, SIGSTOP);
    kill(newpid, SIGCONT);
}

void RunScheduler(void) {
    timer_t timerID; // store timer id

    // set handler of SIGALRM
    // set sigaction
    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = myHandler_SIGALRM;
    sigemptyset(&act.sa_mask);
    sigaction(SIGALRM, &act, NULL);

    // set sigevent
    struct sigevent sevent;
    sevent.sigev_notify = SIGEV_SIGNAL;
    sevent.sigev_signo = SIGALRM;
    sevent.sigev_value.sival_ptr = timerID;

    // create timer
    timer_create(CLOCK_REALTIME, &sevent, &timerID);

    // store interval of timer
    struct itimerspec ts;
    ts.it_interval.tv_sec = TIMESLICE;
    ts.it_interval.tv_nsec = 0;
    ts.it_value.tv_sec = TIMESLICE;
    ts.it_value.tv_nsec = 0;

    // set interval of timer
    timer_settime(timerID, 0, &ts, NULL);
}
