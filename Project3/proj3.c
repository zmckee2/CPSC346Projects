/*
Class: CPSC 346-02
Team Member 1: Zach McKee
Team Member 2: N/A 
GU Username of project lead: zmckee
Pgm Name: proj3.c 
Pgm Desc: exploration of the proc file system 
Usage: 1) standard:  ./a.out -s 
Usage: 2) history:  ./a.out -h 
Usage: 3) load:  ./a.out -l 
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void standard();
void history();
void load();

int main(int argc, char* argv[])
{
 if (argc != 2)
  {
   fprintf(stderr, "Error: Options required\n"); 
   fprintf(stderr, "usage: ./a.out -s|-h|-l\n\n"); 
   exit(EXIT_FAILURE);
  }
 if ((strcmp(argv[1],"-s") != 0) && ((strcmp(argv[1],"-h")) != 0) && ((strcmp(argv[1], "-l")) != 0)) {
	 fprintf(stderr, "Invalid argument received\n");
	 fprintf(stderr, "Valid arguments: -s|-h|-l\n\n");
	 exit(EXIT_FAILURE);
 } 
 
 if (!strcmp(argv[1],"-s"))
  standard();
 if (!strcmp(argv[1],"-h"))
  history();
 if (!strcmp(argv[1],"-l"))
  load();
}
/*
pre: none
post: displays CPU vendor_id, model name, and OS version
*/
void standard()
{
 char ch;
 FILE* ifp;
 char str[80];

 /*
 I've deliberately used two different mechanisms for writing to the console.  
 Use whichever suits you.
 strstr locates a substring
 */

 ifp = fopen("/proc/cpuinfo","r");
 while (fgets(str,80,ifp) != NULL)
  if (strstr(str,"vendor_id") || strstr(str,"model name"))
   puts(str); 
 fclose (ifp);

 ifp = fopen("/proc/version","r");
 while ((ch = getc(ifp)) != EOF)
  putchar(ch);
 fclose (ifp);
}

/*
pre: none
post: displays time since the last reboot (DD:HH:MM:SS), time when the system was last booted 
      (MM/DD/YY - HH:MM), number of processes that have been created since the last reboot.
Hint: strftime could be useful
*/
void history()
{
 FILE* ifp;
 struct tm *formatedBootTime;
 char prettyUptime[80];
 char bootTime[80];
 char processes[80];
 char bufferString[80];
 int days = 0;
 int hours = 0;
 int minutes = 0;
 int seconds = 0;
 
 ifp = fopen("/proc/uptime", "r");
 int uptime = 0;
 fscanf(ifp, "%d", &uptime);
 //To convert the seconds since last launch stored in uptime,
 //I used loops to break uptime down into the individual
 //components needed for displaying the uptime.
 while(uptime > 86400) {
	 days += 1;
	 uptime -= 86400;
 }
 while(uptime > 3600) {
	 hours += 1;
	 uptime -= 3600;
 }
 while(uptime > 60) {
	 minutes += 1;
	 uptime -= 60;
 }
 seconds = uptime;
 
 printf("Time since last reboot: %02d:%02d:%02d:%02d\n",days,hours,minutes,seconds);
 fclose(ifp);
 
 //Looking into the stat folder to get the time
 //since last boot and processes
 ifp = fopen("/proc/stat", "r");
 while(fgets(bufferString,80,ifp) != NULL){
	 if(strstr(bufferString, "btime")) {
		 strncpy(bootTime, bufferString + 6, 74);
	 }
	 if(strstr(bufferString, "processes")) {
		 strncpy(processes, bufferString + 10, 70);
	 }
 }
 int bootTimeInt = atoi(bootTime);
 //Converting the boot time as an int into a
 //format usable by strftime.
 time_t bufferTime = (time_t) bootTimeInt;
 formatedBootTime = localtime(&bufferTime);
 strftime(bootTime, 80, "%x - %H:%M", formatedBootTime);
 printf("Time when system was last rebooted: %s\n", bootTime);
 
 printf("Processes created since last reboot: %s", processes);
 fclose(ifp);
 
}

/*
pre: none
post: displays total memory, available memory, load average (avg. number of processes over the last minute) 
*/
void load()
{
	char memTotal[80];
	char memAvail[80];
	char bufferString[80];
	FILE* memFile;
	
	//Opening the meminfo file
	memFile = fopen("/proc/meminfo", "r");
	//Looping to find the total memory and memory free
	while(fgets(bufferString, 80, memFile) != NULL) {
		if(strstr(bufferString, "MemTotal")) {
			strncpy(memTotal, bufferString + 10, 70);
		}
		if(strstr(bufferString, "MemFree")){
			strncpy(memAvail, bufferString + 9, 71);
		}
	}
	
	//Using while loops and pointers,
	//these two loops remove the spaces
	//in the strings containing the memory
	//stats so they can be converted to ints.
	int i = 0;
	char* memTPointer = memTotal;
	while(memTPointer[0] == ' ') {
		memTPointer++;
	}
	
	char* memAPointer = memAvail;
	while(memAPointer[0] == ' ') {
		memAPointer++;
	}
	
	//Converting the stats to ints
	//made printing easier
	int memTInt = atoi(memTPointer);
	int memAInt = atoi(memAPointer);
	printf("Total memory: %d kB\n", memTInt);
	printf("Available memory: %d kB\n", memAInt);
	
	fclose(memFile);
	//Opening the loadavg file
	memFile = fopen("/proc/loadavg", "r");
	//Becuase I only need the first number in load average,
	//I only need to take the first 5 characters in memfile
	fgets(bufferString, 5, memFile);
	printf("Load average over the last minute: %s\n", bufferString);
	fclose(memFile);
}
