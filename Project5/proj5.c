/*
Class: CPSC 346-02
Team Member 1: Zach McKee
Team Member 2: Kevin Hance
GU Username of project lead: zmckee
Pgm Name: proj5.c
Pgm Desc: This program demonstrates usage of shared memory
Usage: ./a.out
	   OR
	   ./a.out time_parent time_child time_parent_non_cs time_child_non_cs
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/wait.h>


void parent(int, int, int*, int*, int*);
void child(int, int, int*, int*, int*);
void cs(char, int);
void non_crit_sect(int);

int main(int argc, char *argv[])
{
	int tc, tc_ncs, tp, tp_ncs;
	int* turn;
	int* pr_C;
	int* pr_P;
	int shmidT,shmidC,shmidP;

	shmidT = shmget(0,1,0777 | IPC_CREAT);
	shmidC = shmget(0,1,0777 | IPC_CREAT);
	shmidP = shmget(0,1,0777 | IPC_CREAT);

	turn = shmat(shmidT, 0,0);
	pr_C = shmat(shmidC, 0,0);
	pr_P = shmat(shmidP, 0,0);
	*turn = 0;
	*pr_C = 0;
	*pr_P = 0;
	//check for proper arguments
	if(argc > 5 || (argc < 5 && argc > 1) || argc < 1) {
		printf("Invalid argument count, need 0 or 4\n");
		return 0;
	} else if (argc == 1) {
		tc = 1;
		tc_ncs = 1;
		tp = 1;
		tp_ncs = 1;
	} else if (argc == 5) {
		tc = atoi(argv[2]);
		tc_ncs = atoi(argv[4]);
		tp = atoi(argv[1]);
		tp_ncs = atoi(argv[3]);
	}
	//fork here
	int pid = fork();
	if(pid < 0) {
		printf("Error forking");
		return 0;
	} else if (pid == 0) {
		child(tc, tc_ncs, turn, pr_C, pr_P);
		shmdt(turn);
		shmdt(pr_C);
		shmdt(pr_P);
	} else {
		parent(tp, tp_ncs, turn, pr_C, pr_P);
		wait(NULL);
		shmdt(turn);
		shmdt(pr_C);
		shmdt(pr_P);
		shmctl(shmidT, IPC_RMID,0);
		shmctl(shmidP, IPC_RMID,0);
		shmctl(shmidC, IPC_RMID,0);
	}
	/*parBegin
	{
		child(tc, tc_ncs);
		parent(tp, tp_ncs);
	}
	parEnd*/
}

void parent(int tcs, int tncs, int* turn, int* pr_C, int* pr_P)
{
	for (int i = 0; i < 10; i++)
	{
		//protect this
		*pr_P = 1; //Parent is waiting
		*turn = 0; //Give turn to child
		while(*pr_C && (*turn == 0));
		cs('p', tcs);
		non_crit_sect(tncs);
		*pr_P = 0;
	}
}

void child(int tcs, int tncs, int* turn, int* pr_C, int* pr_P)
{
	for (int i = 0; i < 10; i++)
	{
		//protect this
		*pr_C = 1; //Child is waiting
		*turn = 1; //Giving turn to parent
		while(*pr_P && *turn);
		cs('c', tcs);
		non_crit_sect(tcs);
		*pr_C = 0;
	}
}

void cs(char process, int tcs)
{
	if (process == 'p')
	{
		printf("parent in critical section\n");
		sleep(tcs);
		printf("parent leaving critical section\n");
	}
	else
	{
		printf("child in critical section\n");
		sleep(tcs);
		printf("child leaving critical section\n");
	}
}

void non_crit_sect(int tncs)
{
	sleep(tncs);
}
