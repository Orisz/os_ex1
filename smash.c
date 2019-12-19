/*	smash.c
main file. This file contains the main function of smash
*******************************************************************/
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "commands.h"
#include "signals.h"
#include "job_struct.h"

#define MAX_LINE_SIZE 80
#define MAXARGS 20
#define MAX_NUM_OF_JOBS 100
#define MAX_HISTORY_SIZE 50
#define MAX_PATH_LENGTH 80


pjobs_list_t pjobs_list = NULL;
char lastWorkingDir[MAX_PATH_LENGTH];
char lineSize[MAX_LINE_SIZE]; 
char commandHistory[MAX_HISTORY_SIZE][MAX_LINE_SIZE];

//**************************************************************************************
// function name: main
// Description: main function of smash. get command from user and calls command functions
//**************************************************************************************
int main(int argc, char *argv[])
{
    char cmdString[MAX_LINE_SIZE];



	/************************************/
	// Init globals
	/************************************/

	// Signal declaretions
	initSignals();

	// Init history array
	initHistory();

	// init jobs list
	pjobs_list = (pjobs_list_t)malloc(sizeof(jobs_list_t));
	pjobs_list->fgJob = NULL;
	pjobs_list->numOfJobsInBg = 0;
	for (int i = 0; i <MAX_NUM_OF_JOBS ; ++i) {
		pjobs_list->jobs[i] = NULL;
	}

	// Init pwd
	char current_path[MAX_LINE_SIZE];
	if(getcwd(current_path, sizeof(current_path)) == NULL){
		printf("smash error: > getcwd failed");
		exit(1);
	}
	strcpy(lastWorkingDir, current_path);

	/************************************/
	// Main program loop
	/************************************/

	while (1)
	{
		printf("smash > ");
		fgets(lineSize, MAX_LINE_SIZE, stdin);
		strcpy(cmdString, lineSize);
       	updateHistory(cmdString);
		reorderJobList(0);
       	cmdString[strlen(lineSize)-1]='\0';
		// perform a complicated Command
		if(!ExeComp(lineSize)) continue;
		// background command
		if(!BgCmd(lineSize, cmdString)) continue;
		// built in commands
		ExeCmd(lineSize, cmdString);

		/* initialize for next line read*/
		lineSize[0]='\0';
		cmdString[0]='\0';
	}
    return 0;
}

