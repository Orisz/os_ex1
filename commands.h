#ifndef _COMMANDS_H
#define _COMMANDS_H
#include <unistd.h> 
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "job_struct.h"
#include "signals.h"

#define MAX_LINE_SIZE 80
#define MAX_ARG 20
int ExeComp(char* lineSize);
int BgCmd(char* lineSize, char* cmdString);
int ExeCmd(char* lineSize, char* cmdString);
void ExeExternal(char *args[MAX_ARG], char* cmdString, bool isBgCmd);
extern char commandHistory[MAX_HISTORY_SIZE][MAX_LINE_SIZE];
int activeSuspendedJob(pjob_t jobToActive);
void initHistory(void);
void updateHistory(char* command);
extern char commandHistory[MAX_HISTORY_SIZE][MAX_LINE_SIZE];
#endif

