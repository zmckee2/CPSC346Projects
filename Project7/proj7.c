/*
Class: CPSC 346-02
Team Member 1: Zach McKee
Team Member 2: N/A
GU Username of project lead: zmckee
Pgm Name: proj7.c
Pgm Desc: This program demonstrates usage of pipes to communicate
          between threads
Compile using gcc prog7.c -pthread
Usage: ./a.out num_ints
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

//Structure used to pass parameters to all
//thread functions
struct Params {
    int num_ints;
    int* p;
};

typedef struct Params Params;
void* writeToPipe(void* params);
void* readFromPipe(void* params);
void* readFromPipe2(void* params);
void* readFromPipe3(void* params);
int is_prime(int num);

int main(int argc, char* argv[]){
    int status;
    //Check to see if there is a valid
    //number of parameters
    if(argc==0 || argc == 1){
        printf("No aguments supplied.\n");
        printf("Usage: ./a.out num_ints\n");
        exit(1);
    } else if (argc > 2){
        printf("Too many arguments\n");
        printf("Usage: ./a.out num_ints\n");
        exit(1);
    }
    int p[2];
    if (pipe(p) < 0)
        exit(1);

    pthread_t wrt, rd1, rd2, rd3;
    Params wp, r1, r2, r3;
    int num_ints = atoi(argv[1]);
    //Initilize the parameters to be sent to the functions
    //For the writer thread, num_ints is the numbrer of ints
    //to generate.
    //For the reader threads, num_ints is which thread it is
    wp.num_ints = num_ints;
    wp.p = p;
    r1.num_ints = 1;
    r1.p = p;
    r2.num_ints = 2;
    r2.p = p;
    r3.num_ints = 3;
    r3.p = p;

    //Break off to each thread
    status = pthread_create(&wrt, NULL, writeToPipe, (void*)&wp );
    if(status != 0) {
        printf("Error in write thread:  %d\n",status);
        exit(1);
    }
    
    status = pthread_create(&rd1, NULL, readFromPipe, (void*)&r1 );
    if(status != 0) {
        printf("Error in read thread 1:  %d\n",status);
        exit(1);
    }
    status = pthread_create(&rd2, NULL, readFromPipe, (void*)&r2 );
    if(status != 0) {
        printf("Error in read thread 2:  %d\n",status);
        exit(1);
    }
    status = pthread_create(&rd3, NULL, readFromPipe, (void*)&r3 );
    if(status != 0) {
        printf("Error in read thread 3:  %d\n",status);
        exit(1);
    }

    //Bring all the threads back together
    pthread_join(wrt, NULL);
    pthread_join(rd1, NULL);
    pthread_join(rd2, NULL);
    pthread_join(rd3, NULL);
    return 0;
}

void* writeToPipe(void* params)
{
    Params* wp = (Params*) params;  //Get the parameters into the Params
    int num_ints = wp -> num_ints;  //data type
    int* p = wp -> p;
    int numGenerated = 0;
    srand(time(0));                 //Initilize the random number generator
    while(numGenerated < num_ints) {//Generate random numbers until num_int
        int curRand = rand();       //numbers have been generated
        write(p[1], &curRand, sizeof(int));
        printf("Generated %d\n", curRand);
        usleep(300000);
        numGenerated++;
    }
    //Writer is done writing to pipe,
    //Close input end
    close(p[1]);
    pthread_exit(NULL);
}

void* readFromPipe(void* params)
{
    int curIn;
    Params* rp = (Params*) params;              //Convert the parameters into
    int readThread = (*rp).num_ints;            //the Params data type
    int* p = rp -> p;
    while(read(p[0], &curIn, sizeof(int))) {    //Keep reading from the pipe
        //usleep(10000);                          //Unitl it's empty
        if(is_prime(curIn)) {
            printf("Reader %d: %d is prime\n", readThread, curIn);
            sleep(1);
        }
    }
    pthread_exit(NULL);
}

//Checks to see if a number is prime
int is_prime(int num)
{
    int i = 2;
    while (i < num)
    {
        if (num % i == 0)
            return 0;
        ++i;
    }
    return 1;
} 