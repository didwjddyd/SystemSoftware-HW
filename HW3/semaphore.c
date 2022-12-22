#include <stdio.h>
#include <stdlib.h>
#include "thread.h"
#include "semaphore.h"

#include <string.h>


SemaphoreTblEnt pSemaphoreTblEnt[MAX_SEMAPHORE_NUM];

int thread_sem_open(char* name, int count) {
    int i;
    // check name of semaphore is already exist
    for(i = 0; i < MAX_SEMAPHORE_NUM; ++i) {
        SemaphoreTblEnt entry = pSemaphoreTblEnt[i];
        if(strcmp(entry.name, name) == 0 && entry.bUsed == 1) {
            return i; // return semaphore id
        }
    }

    // create new semaphore
    for(i = 0; i < MAX_SEMAPHORE_NUM; ++i) {
        SemaphoreTblEnt *entry = &pSemaphoreTblEnt[i];
        if(entry->bUsed == 0) {
            // set name, bUsed
            strcpy(entry->name, name);
            entry->bUsed = 1;

            // allocate Semaphore
            entry->pSemaphore = malloc(sizeof(Semaphore));

            // set token of Semaphore
            entry->pSemaphore->count = count;

            // initalize waitingQueue of Semaphore
            entry->pSemaphore->waitingQueue.queueCount = 0;
            entry->pSemaphore->waitingQueue.pHead = NULL;
            entry->pSemaphore->waitingQueue.pTail = NULL;
            return i; // return semaphore id
        }
    }
}

// temp:
// signal handler of SIGUSR1
// for thread_sem_post() and signal_sem_wait()
void myHandler_SIGUSR1(int signum) {}

int thread_sem_wait(int semid) {
    Semaphore *sem = pSemaphoreTblEnt[semid].pSemaphore;

    int tid = thread_self();

    // temp: set signal
    signal(SIGUSR1, myHandler_SIGUSR1);

    // check sem->count == 1
    if(sem->count > 0) {
        sem->count--;
        return 0;
    }
    else {
        // sem->count == 0
        // put current thread into waitingQueue of Sempahore
        pCurrentThread->pNext = NULL;
        pCurrentThread->pPrev = NULL;

        //memo: think about pop blocked thread from ready queue
        if(pCurrentThread->status != THREAD_STATUS_ZOMBIE) {
            pCurrentThread->status = THREAD_STATUS_WAIT;
        }

        if(sem->waitingQueue.queueCount == 0) {
            sem->waitingQueue.pHead = pCurrentThread;
            sem->waitingQueue.pTail = pCurrentThread;

            sem->waitingQueue.queueCount++;
        }
        else {
            sem->waitingQueue.pTail->pNext = pCurrentThread;
            pCurrentThread->pPrev = sem->waitingQueue.pTail;

            sem->waitingQueue.pTail = sem->waitingQueue.pTail->pNext;

            sem->waitingQueue.queueCount++;
        }

        pCurrentThread = ReadyQueue.pHead;
        ReadyQueue.queueCount--;
    }

PAUSE:
    pause(); // temp: waiting signal of thread_sem_post()

    // temp: re-check sem->count == 1
    if(sem->count > 0) {
        // temp: put head of waitingQueue into ReadyQueue
        Thread *target = sem->waitingQueue.pHead;

        target->status = THREAD_STATUS_READY;

        if(sem->waitingQueue.queueCount == 1) {
            sem->waitingQueue.pHead = NULL;
            sem->waitingQueue.pTail = NULL;

            sem->waitingQueue.queueCount--;
        }
        else {
            // sem->waitingQueue.queueCount > 1
            sem->waitingQueue.pHead = target->pNext;
            sem->waitingQueue.pHead->pPrev = NULL;

            sem->waitingQueue.queueCount--;
        }

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

        sem->count--;

        return 0;
    }
    else {
        // sem->count == 0
        // temp: pause again
        goto PAUSE;
    }
}

int thread_sem_post(int semid) {
    Semaphore *sem = pSemaphoreTblEnt[semid].pSemaphore;

    sem->count++;

    // temp: send signal to head of waitingQueue
    if(sem->waitingQueue.queueCount > 0) {
        kill(sem->waitingQueue.pHead->pid, SIGUSR1);
    }

    return 0;
}

int thread_sem_close(int semid) {
    SemaphoreTblEnt *target = &pSemaphoreTblEnt[semid];

    target->name[0] = '\0';
    target->bUsed = 0;
    free(target->pSemaphore);
}
