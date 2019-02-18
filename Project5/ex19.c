#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
 void   child(char*);
 void   parent(char*,int);
 char*  address;
 int    shmid, value;

 //create a shared memory segment
 shmid = shmget(0,1,0777 | IPC_CREAT);

 //attach it to the process, cast its address, and 
 address = (char*)shmat(shmid,0,0); 

 //initialize it to 'A'
 *address = 'A';
 printf("Initial value of shared memory segment: %c.\n",*address);

 if (fork() == 0)
    child(address);
 else 
    parent(address,shmid);
}

void parent(char* address, int shmid)
 {
   /*block, briefly, so that child runs first*/
   sleep(1);

   /*display current value of shared memory*/
   printf("Parent holds new value: %c.\n",*address);
   
   //Wait for child to exit
   wait(NULL);

   //detach shared memory from the process
   shmdt(address);
   
   //remove it 
   shmctl(shmid,IPC_RMID,0);
 }
 
void child(char* address)
 {
   //display the current value of shared memory
   printf("Child holds shared memory value: %c\n",*address);

   //increment it
   (*address)++;
   printf("Child has incremented the value: %c\n",*address);

   //detach the shared memory from the process*/
   shmdt(address);
 }
   
