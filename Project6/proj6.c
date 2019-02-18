/*
Class: CPSC 346-02
Team Member 1: Zach McKee
Team Member 2: N/A
GU Username of project lead: zmckee
Pgm Name: proj6.c
Pgm Desc: This program demonstrates usage of semaphores to solve producer consumer problem
Usage: ./a.out
*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define init_mutex 1
#define init_full  0
#define init_empty 100
#define empty_arg  0
typedef struct sembuf sem_buf;

void producer(int,int,int, sem_buf[], sem_buf[]);
void consumer(int,int,int, sem_buf[], sem_buf[]);
void wait_sem(int, sem_buf[]);
void signal_sem(int, sem_buf[]);
void criticalSection(int);
void printSemValues(int,int,int);

int main(int argc, char *argv[])
{
    sem_buf sem_wait[1], sem_signal[1];
    int mutex, full, empty;
    int forkValue, status, ch_stat;

    //set the buffer values for wait.
    sem_wait[0].sem_num = 0;
    sem_wait[0].sem_op = -1; //decrement
    sem_wait[0].sem_flg = SEM_UNDO;

    //set the buffer values for signal
    sem_signal[0].sem_num = 0;
    sem_signal[0].sem_op = 1; //increment
    sem_signal[0].sem_flg = SEM_UNDO;

    //Create all the buffers
    if (((mutex = semget(0, 1, 0777 | IPC_CREAT)) == -1) || 
        ((full = semget(1, 1, 0777 | IPC_CREAT)) == -1) ||
        ((empty = semget(2, 1, 0777 | IPC_CREAT)) == -1))
    {
        printf("semget failed\n");
        exit(1);
    }
    //Initilize buffer values
    semctl(mutex, 0, SETVAL, init_mutex);
    semctl(empty, 0, SETVAL, init_empty);
    semctl(full, 0, SETVAL, init_full);

    printf("Initial semaphore values: mutex=%d, empty=%d, full=%d\n", semctl(mutex,0,GETVAL,0),semctl(empty,0,GETVAL,0),semctl(full,0,GETVAL,0));
    //Par begin
    if((forkValue = fork()) < 0) {
        printf("Error forking\n");
        exit(1);
    }
    else {
        if(forkValue == 0)
            consumer(mutex, empty, full, sem_wait, sem_signal);
        else
        {
            producer(mutex, empty, full, sem_wait, sem_signal);
            //Wait for child and clean up semaphores
            status = wait(&ch_stat);
            semctl(mutex, 1, IPC_RMID, 0);
            semctl(empty, 1, IPC_RMID, 0);
            semctl(full, 1, IPC_RMID, 0);
            
        }
    }
    //Par End, print final semaphore values
    if(forkValue == 0) {
        sleep(1);
        printf("Final semaphore values : mutex=%d, empty=%d, full=%d\n", semctl(mutex,0,GETVAL,0),semctl(empty,0,GETVAL,0),semctl(full,0,GETVAL,0));
    }
}

void producer(int mutex,int empty,int full, sem_buf sem_wait[], sem_buf sem_signal[])
{
    for(int i = 0; i < 5; i++) 
    {
        wait_sem(empty, sem_wait);
        wait_sem(mutex, sem_wait);
        criticalSection(1);
        signal_sem(mutex, sem_signal);
        signal_sem(full, sem_signal);
        sleep(1);
    }
}

void consumer(int mutex,int empty,int full, sem_buf sem_wait[], sem_buf sem_signal[])
{
    for(int i = 0; i < 5; i++) 
    {
        wait_sem(full, sem_wait);
        wait_sem(mutex, sem_wait);
        criticalSection(0);
        signal_sem(mutex, sem_signal);
        signal_sem(empty, sem_signal);
        sleep(2);
    }
}

void wait_sem(int sem, sem_buf semOp[])
{
    while(semctl(sem,0,GETVAL,0) <= 0);
    semop(sem,semOp,1);
}

void signal_sem(int sem, sem_buf semOp[])
{
    semop(sem, semOp, 1);
}

void criticalSection(int who)
{
    if(who == 1)
        printf("Producer making an item\n");
    else
        printf("Consumer consuming an item\n");
}