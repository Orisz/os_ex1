#include "signals.h"
// contains signal handler funtions
// contains the function/s that set the signal handlers


//! **************************************************************************************
/// function name: sendSignal
/// Description: Send signal to a pid according to the user input, prints what signal was sent to the pid
/// Parameters: int pid, int signum
/// Returns: 0 for success, 1 for fail
//! **************************************************************************************
int sendSignal(int pid, int signum)
{
    if (kill(pid, signum) == -1) {
        return -1;
    }

    printf("smash: > signal %s was sent to pid %d\n", sigName(signum), pid);
    return 0;
}


//! **************************************************************************************
/// function name: initSignals
/// Description: init the signal handler for SIGINT and SIGSTP using the sigaction struct
/// Parameters: NONE
/// Returns: void
//! **************************************************************************************
void initSignals(){

	struct sigaction sigintAct, sigtstpAct;
	sigintAct.sa_handler = &sigIntHandler;
	sigfillset(&sigintAct.sa_mask);
	sigintAct.sa_flags = 0;
	sigtstpAct.sa_handler = &sigStopHandler;
	sigfillset(&sigtstpAct.sa_mask);
	sigtstpAct.sa_flags = 0;
	if ((sigaction(SIGINT, &sigintAct, NULL)) || (sigaction(SIGTSTP, &sigtstpAct, NULL)))
	{
		perror("smash error: > failed to initialize sigaction()");
		exit(1);
	}
}


//! **************************************************************************************
/// function name: sigStopHandler
/// Description: Ctrl z - stop the process in the FG, and pass it to the BG
/// Parameters: x, never used
/// Returns: void
//! **************************************************************************************
void sigStopHandler(int x) {
	pjob_t fg_job = pjobs_list->fgJob;
	if (fg_job == NULL) {
		return;
	}
	else {
		if (sendSignal(fg_job->pid, SIGTSTP) != 0) {
			perror("Send signal failed");
		}
		else {
			fg_job->is_running = FALSE;
			pjob_t isJobOnJobList = getJobByPid(fg_job->pid); //possible if job moved to fg by the "fg" cmd
			if(isJobOnJobList == NULL){
				//job is not on the fg
				insertJobToJobList(fg_job);
			}
			
			pjobs_list->lastSuspendedPid = fg_job->pid;
			pjobs_list->fgJob = NULL;
		}
		
	}
}


//! **************************************************************************************
/// function name: sigIntHandler
/// Description: Ctrl c - terminate the process in the FG
/// Parameters: x, never used
/// Returns: void
//! **************************************************************************************
void sigIntHandler(int x) {
	pjob_t fg_job = pjobs_list->fgJob;
	if (fg_job == NULL) {
		return;
	}
	else {
		if (sendSignal(fg_job->pid, SIGINT) != 0) {
			//fail to terminate the process...
			perror("Send signal failed");
		}
		else {
            // Wait for the child to dieeeee
            waitpid(fg_job->pid, NULL, WUNTRACED);
            // no need to delete using "deleteJob" func because the job was never
			// on the list - it was running on the FG and delete reorders the list.
            free(fg_job);
			// process was killed, update global FG job
			pjobs_list->fgJob = NULL;
		}
	}
}


//! **************************************************************************************
/// function name: sigName
/// Description: Get the signal name by the signal number
/// Parameters: int signum
/// Returns: signal name
//! **************************************************************************************
//
char* sigName(int signum)
{
    switch (signum)
    {
        case 1: return "SIGHUP";
        case 2: return "SIGINT";
        case 3: return "SIGQUIT";
        case 4: return "SIGILL";
        case 5: return "SIGTRAP";
        case 6: return "SIGIOT";
        case 7: return "SIGBUS";
        case 8: return "SIGFPE";
        case 9: return "SIGKILL";
        case 10: return "SIGUSR1";
        case 11: return "SIGSEGV";
        case 12: return "SIGUSR2";
        case 13: return "SIGPIPE";
        case 14: return "SIGALRM";
        case 15: return "SIGTERM";
        case 16: return "SIGSTKFLT";
        case 17: return "SIGCHLD";
        case 18: return "SIGCONT";
        case 19: return "SIGSTOP";
        case 20: return "SIGTSTP";
        case 21: return "SIGTTIN";
        case 22: return "SIGTTOU";
        case 23: return "SIGURG";
        case 24: return "SIGXCPU";
        case 25: return "SIGXFSZ";
        case 26: return "SIGVTALRM";
        case 27: return "SIGPROF";
        case 28: return "SIGWINCH";
        case 29: return "SIGIO";
        case 30: return "SIGPWR";
        case 31: return "SIGSYS";
        default: return "_UNKNOWN_SIGNAL_";
    }
}
