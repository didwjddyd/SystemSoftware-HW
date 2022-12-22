#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include "init.h"
#include "thread.h"
#include "scheduler.h"

ThreadQueue ReadyQueue;
ThreadQueue WaitingQueue;
ThreadTblEnt pThreadTbEnt[MAX_THREAD_NUM];
Thread* pCurrentThread; // Running ������ Thread�� ����Ű�� ����


int thread_create(thread_t *thread, thread_attr_t *attr, void *(*start_routine)(void*), void *arg) {
    int flags = CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM; // flag of clone()

    pid_t newPid;
    void *newStackAddr = malloc(1024 * 64);

    newPid = clone((void *)start_routine, newStackAddr + 1024 * 64, flags, arg);
    kill(newPid, SIGSTOP);

    // allocate TCB
    Thread *tcb; // create thread contol block
    tcb = malloc(sizeof(Thread)); // memory allocation

    // initialize TCB
    tcb->stackSize = 1024 * 64;
    tcb->stackAddr = newStackAddr;
    tcb->status = THREAD_STATUS_READY; // set status to ready
    tcb->pid = newPid;
    tcb->cpu_time = 0;
    tcb->pPrev = NULL;
    tcb->pNext = NULL;

    // move TCB to ready queue
    if(ReadyQueue.queueCount == 0) {
        ReadyQueue.pHead = tcb;
        ReadyQueue.pTail = tcb;
        ReadyQueue.queueCount++;
    }
    else {
        tcb->pPrev = ReadyQueue.pTail;
        ReadyQueue.pTail->pNext = tcb;

        ReadyQueue.pTail = tcb;

        ReadyQueue.queueCount++;
    }

    // get thread id
    int i;
    for (i = 0; i < MAX_THREAD_NUM; ++i) {
        if(pThreadTbEnt[i].bUsed == 0) {
            pThreadTbEnt[i].bUsed = 1;
            pThreadTbEnt[i].pThread = tcb;
            *thread = i;
            break;
        }
    }

    return *thread; // return thread id
}

int thread_suspend(thread_t tid) {
    Thread *target = pThreadTbEnt[tid].pThread;

    // move TCB to waiting queue
    // take off TCB from queue
    switch(target->status) {
        case THREAD_STATUS_READY:
            // check target is head
            if(ReadyQueue.pHead == target) {
                ReadyQueue.pHead = ReadyQueue.pHead->pNext;
            }
            else if(target->pPrev != NULL) {
                target->pPrev->pNext = target->pNext;
            }

            // check target is tail
            if(ReadyQueue.pTail == target) {
                ReadyQueue.pTail = ReadyQueue.pTail->pPrev;
            }
            else if(target->pNext != NULL) {
                target->pNext->pPrev = target->pPrev;
            }

            target->pNext = NULL;
            target->pPrev = NULL;

            ReadyQueue.queueCount--;

            break;

        case THREAD_STATUS_WAIT:
            // exclude target is tail
            if(WaitingQueue.pTail != target) {
                // check target is head
                if(WaitingQueue.pHead == target) {
                    WaitingQueue.pHead = target->pNext;
                }
                else {
                    target->pPrev->pNext = target->pNext;
                }
                target->pNext = NULL;
                target->pPrev = NULL;
            }

            WaitingQueue.queueCount--;

            break;
    }

    // push back TCB into waiting queue
    if(WaitingQueue.queueCount == 0) {
        WaitingQueue.pHead = target;
        WaitingQueue.pTail = target;
        WaitingQueue.queueCount++;
    }
    else {
        WaitingQueue.pTail->pNext = target;
        target->pPrev = WaitingQueue.pTail;

        WaitingQueue.pTail = WaitingQueue.pTail->pNext;

        WaitingQueue.queueCount++;
    }

    // set thread status to waiting
    target->status = THREAD_STATUS_WAIT;

    return 0;
}

int thread_cancel(thread_t tid) {
    Thread *target = pThreadTbEnt[tid].pThread;

    // kill target thread
    if(ReadyQueue.queueCount != 0) {
        kill(target->pid, SIGKILL);
    }

    // remove TCB from ready or waiting queue
    switch(target->status) {
        case THREAD_STATUS_ZOMBIE:
        case THREAD_STATUS_READY:
            // check target is head
            if(ReadyQueue.pHead == target) {
                ReadyQueue.pHead = ReadyQueue.pHead->pNext;
            }
            else if(target->pPrev != NULL) {
                target->pPrev->pNext = target->pNext;
            }

            // check target is tail
            if(ReadyQueue.pTail == target) {
                ReadyQueue.pTail = ReadyQueue.pTail->pPrev;
            }
            else if(target->pNext != NULL) {
                target->pNext->pPrev = target->pPrev;
            }

            target->pNext = NULL;
            target->pPrev = NULL;

            ReadyQueue.queueCount--;

            break;

        case THREAD_STATUS_WAIT:
            // check target is head
            if(WaitingQueue.pHead == target) {
                WaitingQueue.pHead = WaitingQueue.pHead->pNext;
            }
            else {
                target->pPrev->pNext = target->pNext;
            }

            // check target is tail
            if(WaitingQueue.pTail == target) {
                WaitingQueue.pTail = WaitingQueue.pTail->pPrev;
            }
            else {
                target->pNext->pPrev = target->pPrev;
            }

            target->pNext = NULL;
            target->pPrev = NULL;

            WaitingQueue.queueCount--;

            break;
    }

    //deallocate target thread TCB
    // free
    free(target->stackAddr);
    free(target);

    // reset pThreadTbEnt[tid]
    pThreadTbEnt[tid].bUsed = 0;
    pThreadTbEnt[tid].pThread = NULL;

    return 0;
}

int thread_resume(thread_t tid) {
    Thread *target = pThreadTbEnt[tid].pThread;

    // set target thread status to ready
    target->status = THREAD_STATUS_READY;

    // move TCB from waiting queue to ready queue
    // take off TCB from waiting queue
    // check target is head
    if(WaitingQueue.pHead == target) {
        WaitingQueue.pHead = WaitingQueue.pHead->pNext;
    }
    else {
        target->pPrev->pNext = target->pNext;
    }

    // check target is tail
    if(WaitingQueue.pTail == target) {
        WaitingQueue.pTail = WaitingQueue.pTail->pPrev;
    }
    else {
        target->pNext->pPrev = target->pPrev;
    }
    target->pNext = NULL;
    target->pPrev = NULL;

    WaitingQueue.queueCount--;

    // push back TCB into ready queue
    if(ReadyQueue.queueCount == 0) {
        ReadyQueue.pHead = target;
        ReadyQueue.pTail = target;
        ReadyQueue.queueCount++;
    }
    else {
        ReadyQueue.pTail->pNext = target;
        target->pPrev = ReadyQueue.pTail;

        ReadyQueue.pTail = ReadyQueue.pTail->pNext;

        ReadyQueue.queueCount++;
    }

    return 0;
}

thread_t thread_self(void) {
    int i;
    for(i = 0; i < MAX_THREAD_NUM; ++i) {
        if(pThreadTbEnt[i].pThread == pCurrentThread) {
            break;
        }
    }

    return i;
}

// just handling SIGCHLD
void myHandler_SIGCHLD(int signum) {}

int thread_join(thread_t tid) {
    // set zombie reaping handler of SIGCHLD
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = myHandler_SIGCHLD;
    sigaction(SIGCHLD, &act, NULL);

    // move current TCB info waiting queue
    thread_suspend(0);

    // select new thread to run on CPU
    Thread *newThread = pThreadTbEnt[tid].pThread;

    if(newThread->status == THREAD_STATUS_ZOMBIE) {
        goto REAPING;
    }

    // remove new thread TCB from ready queue
    if(newThread != pCurrentThread && ReadyQueue.queueCount == 1) {
        ReadyQueue.pHead = NULL;
        ReadyQueue.pTail = NULL;
        ReadyQueue.queueCount--;
    }
    else if (newThread != pCurrentThread && ReadyQueue.queueCount > 1) {
        if(newThread == ReadyQueue.pHead) {
            ReadyQueue.pHead = ReadyQueue.pHead->pNext;
            ReadyQueue.pHead->pPrev = NULL;
        }
        else {
            Thread *perv = newThread->pPrev;
            newThread->pPrev->pNext = newThread->pNext;
        }

        if(newThread == ReadyQueue.pTail) {
            ReadyQueue.pTail = ReadyQueue.pTail->pPrev;
            ReadyQueue.pTail->pNext = NULL;
        }
        else {
            Thread *next = newThread->pNext;
            next->pPrev = newThread->pPrev;
        }

        ReadyQueue.queueCount--;
    }

    newThread->pNext = NULL;
    newThread->pPrev = NULL;

    // set status to run
    if(newThread->status != THREAD_STATUS_ZOMBIE) {
        newThread->status = THREAD_STATUS_RUN;
    }

    pCurrentThread = newThread;
    kill(newThread->pid, SIGCONT);

PAUSE:
    pause();

    // zombie reaping
    // move current thread TCB into ready queue
    // set status to ready
REAPING:
    thread_resume(0);

    if(newThread->status == THREAD_STATUS_ZOMBIE) {
        kill(newThread->pid, SIGKILL);
        free(newThread->stackAddr);
        free(newThread);

        pThreadTbEnt[tid].bUsed = 0;
        pThreadTbEnt[tid].pThread = NULL;

        return 0;
    }
    else {
        thread_suspend(0);
        goto PAUSE;
    }
}

int thread_cputime(void) {
    return pCurrentThread->cpu_time;
}

void Init(void) {
    // initialize thread table
    int i;
    for (i = 0; i < MAX_THREAD_NUM; ++i) {
        pThreadTbEnt[i].bUsed = 0;
        pThreadTbEnt[i].pThread = NULL;
    }

    // initialize ready queue
    ReadyQueue.queueCount = 0;
    ReadyQueue.pHead = NULL;
    ReadyQueue.pTail = NULL;

    // initialize waiting queue
    WaitingQueue.queueCount = 0;
    WaitingQueue.pHead = NULL;
    WaitingQueue.pTail = NULL;
}

void thread_exit(void) {
    pCurrentThread->status = THREAD_STATUS_ZOMBIE;

    if(ReadyQueue.queueCount == 0) {
        pCurrentThread = NULL;
        kill(pThreadTbEnt[0].pThread->pid, SIGCHLD);
    }
    else {
        pCurrentThread = ReadyQueue.pHead;
        ReadyQueue.pHead = ReadyQueue.pHead->pNext;
        ReadyQueue.queueCount--;
        kill(pThreadTbEnt[0].pThread->pid, SIGCHLD);
    }
}
