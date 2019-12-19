#ifndef _SIGS_H
#define _SIGS_H
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "job_struct.h"

// Get pid and signal number, send the right signal and print what was sent to the screen
int sendSignal(int pid, int signum);

// Initialize the signals handlers
void initSignals();

/// Ctrl z - stop the process in the FG, and pass it to the BG
void sigStopHandler(int x);

/// Ctrl c - terminate the process in the FG
void sigIntHandler(int x);

//return the name of the signal
char* sigName(int signum);

#endif

