/*
Class: CPSC 346-02
Team Member 1: Zach McKee
Team Member 2: Kevin Hance
GU Username of project lead: zmckee
Pgm Name: proj4.c
Pgm Desc: A functional shell capable of runnning system commands
Usage: ./a.out
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_LINE 80
#define TRUE 80

char **getInput();
char **parseInput(char *);
void dispOutput(char **);
void dispHistory(char **);

int numArgs;
int numHisArgs;
char *lastInput;

int main(int argc, char *argv[])
{
	//A pointer to an array of pointers to char.  In essence, an array of
	//arrays.  Each of the second level arrays is a c-string. These hold
	//the command line arguments entered by the user.
	//as exactly the same structure as char* argv[]
	char **args;
	numHisArgs = 0;
	char **historyArgs = calloc(10, sizeof(char *));
	for (int i = 0; i < 10; i++)
	{
		historyArgs[i] = calloc(80, sizeof(char *));
	}

	while (TRUE)
	{
		int status;
		int childBreak = 0;
		printf("myShell> ");
		fflush(stdout);
		args = getInput();
		//Ignore if user enters nothing
		if(args != NULL) {
			int waitForChild = 1;
			//Look through the arguments list to see if
			//the user entered a & and respond accordinly.
			for (int i = 0; i < numArgs; i++)
			{
				if (strcmp("&", args[i]) == 0)
					waitForChild = 0;
			}

			dispOutput(args);
			//if the user has entered "quit" break out of the loop.
			if (strcmp("quit", args[0]) == 0)
			{
				break;
			}
			//change directory if the user enters cd
			else if (strcmp("cd", args[0]) == 0) 
			{
				chdir(args[1]);
			}
			//display the history if the user enters history
			else if (strcmp("history", args[0]) == 0)
			{
				dispHistory(historyArgs);
			} 
			//Check to see if the user wants to run a
			//command from history
			else if (*args[0] == '!') 
			{
				char **commandToRun;
				int validNumber = 0;
				if (*(args[0] + 1) == '!')
				{
					if(numHisArgs > 0) {
						commandToRun = parseInput(historyArgs[numHisArgs - 1]);
						validNumber = 1;
					}
					else
					{
						printf("No arguments in history\n");
						validNumber = 0;
					}
				}
				else
				{
					int numOfCommand = *(args[0] + 1) - '0';
					if (numOfCommand > -1 && numOfCommand <= numHisArgs)
					{
						commandToRun = parseInput(historyArgs[numOfCommand-1]);
						validNumber = 1;
					}
					else
					{
						printf("Please pick a number between 0 and %d\n", numHisArgs);
						validNumber = 0;
					}
				}
				if (validNumber == 1)
				{
					for (int i = 0; i < numArgs; i++)
					{
						if (strcmp("&", commandToRun[i]) == 0)
							waitForChild = 0;
					}
					int pid = fork();
					if (pid < 0)
					{
						printf("Error creating child");
					}
					else if (pid == 0)
					{
						//In child program
						execvp(commandToRun[0], commandToRun);
						childBreak = 1;
					}
					else
					{
						//Parent program
						if (waitForChild == 1)
						{
							//No & was passed, wait for child
							wait(&status);
						}
					}
				}
			//The user has entered a command, run it
			} else {
				int pid = fork();
				if (pid < 0)
				{
					printf("Error creating child");
				}
				else if (pid == 0)
				{
					//In child program
					execvp(args[0], args);
					childBreak = 1;
				}
				else
				{
					//Parent program
					if (waitForChild == 1)
					{
						//No & was passed, wait for child
						wait(&status);
					}
				}
				//See figure 3.36 on p. 158.  Do items 1, 2, 3
				//Check to see if the historical arguments
				//array is already at 10
				//If it isn't, add it to the list
				if (numHisArgs != 10)
				{
					historyArgs[numHisArgs] = lastInput;
					numHisArgs++;
				}
				//If the array is full, shift all other arugments
				//down and add the new one to the end
				else
				{
					for (int i = 1; i < 10; i++)
					{
						historyArgs[i - 1] = historyArgs[i];
					}
					historyArgs[9] = lastInput;
				}
			}
		}
		//If a child program is running, end it
		if(childBreak == 1) {
			break;
		}
	}
	return 0;
}

/*
Reads input string from the key board.   
invokes parseInput and returns the parsed input to main
*/
char **getInput()
{
	char *inputString = (char *)malloc(MAX_LINE);
	char *parseString = inputString;
	char curChar;

	while ((curChar = getchar()) != '\n')
		*inputString++ = curChar;
	*inputString = '\0';
	lastInput = parseString;
	return parseInput(parseString);
}

/*
inp is a cstring holding the keyboard input
returns an array of cstrings, each holding one of the arguments entered at
the keyboard. The structure is the same as that of argv
Here the user has entered three arguements:
myShell>cp x y
*/
char **parseInput(char *inp)
{
	//Check to see if the user entered nothing,
	//if they did return NULL
	if(*inp == '\0')
		return NULL;
	char *argsCount = inp;
	numArgs = 0;
	int inArg = 0;
	//Loop through the arugment and see how many
	//arguments there are
	while (*argsCount)
	{
		if (*argsCount != ' ' && inArg != 1)
		{
			numArgs++;
			inArg = 1;
		}
		else if (*argsCount == ' ')
			inArg = 0;
		*argsCount++;
	}
	//Allocate enough memory for each argument and a NULL
	char **args = calloc(numArgs + 1, sizeof(char *));
	for (int i = 0; i < numArgs + 1; i++)
	{
		args[i] = calloc(80, sizeof(char *));
	}
	char *curArg = (char *)malloc(80);
	char *temp = curArg;
	char **tempB = args;
	int curLen = 0;
	int i = 0;
	//While inp points to a character loop through the input string
	while (*inp)
	{
		//If the character is not a space, still in an arg
		//keep track of how long the current arugment is.
		if (*inp != ' ')
		{
			*curArg++ = *inp++;
			curLen++;
		}
		//If the character is a space, add null terminator
		//and copy the string into the args array
		else
		{
			*curArg = '\0';
			strcpy(args[i], temp);
			curArg -= curLen;
			curLen = 0;
			i++;
			*inp++;
		}
	}
	*curArg = '\0';
	strcpy(args[i], temp);
	//Add a null as the last argument to format in a way C likes
	args[numArgs] = NULL;
	return args;
}


/*
Displays the arguments entered by the user and parsed by getInput
*/
void dispOutput(char **args)
{
	for(int i = 0; i < numArgs; i++)
	{
		printf("args[%d] = \"%s\"\n", i, args[i]);
	}
}

void dispHistory(char **args)
{
	for (int i = numHisArgs - 1; i >= 0; i--)
	{
		printf("%d %s\n", i + 1, args[i]);
	}
}