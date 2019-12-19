#include "commands.h"

//! ********************************************
// function name: ExeCmd
// Description: interperts and executes built-in commands
// Parameters: pointer to jobs, command string
// Returns: 0 - success,1 - failure
//! ********************************************
int ExeCmd(char* lineSize, char* cmdString)
{
	char* cmd; 
	char* args[MAX_ARG];
	//char pwd[MAX_LINE_SIZE];
	char* delimiters = " \t\n";  
	int i = 0, num_arg = 0;
	bool illegal_cmd = FALSE; // illegal command
    	cmd = strtok(lineSize, delimiters);
	if (cmd == NULL)
		return 0; 
   	args[0] = cmd;
	for (i=1; i<MAX_ARG; i++)
	{
		args[i] = strtok(NULL, delimiters); 
		if (args[i] != NULL) 
			num_arg++;
	}

/*************************************************/
// Built in Commands PLEASE NOTE NOT ALL REQUIRED
// ARE IN THIS CHAIN OF IF COMMANDS. PLEASE ADD
// MORE IF STATEMENTS AS REQUIRED
/*************************************************/
	if (!strcmp(cmd, "cd") ) 
	{
		int return_val;

		if(num_arg > 1){
			illegal_cmd = TRUE;
		}
		else{
			// save the prev PWD
			char prePath[MAX_LINE_SIZE];

			if(getcwd(prePath, sizeof(prePath)) == NULL){
				printf("smash error: > getcwd failed");
				return 1;
			}

			// return to the last working dir
			if(!strcmp(args[1], "-")){
				return_val = chdir(lastWorkingDir);
			}

			// change dir to what ever
			else{
				return_val = chdir(args[1]);
			}

			// error in change dir
			if(return_val == -1){
				printf("smash error: > %s - path not found \n", args[1]);
				return 1;
			}
			// update last working dir
			strcpy(lastWorkingDir, prePath);

			// get the PWD
			char currentPath[MAX_LINE_SIZE];

			// print the new working dir
			if(getcwd(currentPath, sizeof(currentPath)) == NULL){
				printf("smash error: > getcwd failed");
				return 1;
			}
			printf("%s\n", currentPath);

			return 0;
		}
	}

	
	/*************************************************/
	else if (!strcmp(cmd, "pwd")) 
	{
		char currentPath[MAX_LINE_SIZE];

		if(num_arg > 0){
			illegal_cmd = TRUE;
		}
		else{
			// get the PWD
			if(getcwd(currentPath, sizeof(currentPath)) == NULL){
				printf("smash error: > getcwd failed");
				return 1;
			}
			// print the present working dir
			printf("%s\n", currentPath);
			return 0;
		}
	}
	
	/*************************************************/
	else if (!strcmp(cmd, "mv"))
	{
		if(num_arg != 2){
			illegal_cmd = TRUE;
		}
		else{
			// try to change the name;
			if(rename(args[1], args[2]) == 0){
				printf("%s has been renamed to: %s \n", args[1], args[2]);
				return 0;
			}
			else {
				perror("smash: > rename failed");
				return 1;
			}
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "jobs")) 
	{
		if(num_arg != 0){
			illegal_cmd = TRUE;
		}
		else{
			for (int i=0; i<pjobs_list->numOfJobsInBg; ++i){
                pjob_t currJob = pjobs_list->jobs[i];

                if (currJob == NULL){
                    continue;
                }
				time_t now = time(NULL);
				int runningTime = now - currJob->process_start_time;
				printf("[%d] %s : %d %d secs", currJob->id, currJob->command, currJob->pid, runningTime);

				if(currJob->is_running){
					printf("\n");
				}
				else{
					printf(" (Stopped)\n");
				}
			}
			return 0;
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "showpid")) 
	{
		if(num_arg != 0){
			illegal_cmd = TRUE;
		}
		else{
			printf("smash pid is %d \n", getpid());
			return 0;
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "fg")) 
	{
		pjob_t jobToMove;
		if(num_arg > 1){
			illegal_cmd = TRUE;
		}
		else {
			// Got job id from user, move the corresponding job to the FG
			if (num_arg == 1){
				int jobId = atoi(args[1]);
				// get the job, and verify the idx is legal
				if((jobToMove = getJobById(jobId)) == NULL){
					illegal_cmd = TRUE;
				}
			}
			// Got no Id from user, move the newest job ing BG to the FG
			else {
				if(pjobs_list->numOfJobsInBg == 0){
					printf("smash error: > There are no jobs to move from BG to FG");
					return 1;
				}
				jobToMove = pjobs_list->jobs[pjobs_list->numOfJobsInBg-1];
			}

			// if process is suspended send SIGCONT
			if (illegal_cmd == FALSE){
				// If the job is suspended, activate it in the BG and then move it to the FG
				if(!jobToMove->is_running){
					if(activeSuspendedJob(jobToMove) != 0){
						perror("smash: > Send signal failed");
						return 1;
					}
				}

				pjobs_list->fgJob = jobToMove;
				// move to FG - wait for the process to end
				waitpid(jobToMove->pid, NULL, WUNTRACED);
				if(pjobs_list->fgJob == NULL){
					// a handler took care of the fg job(insertions to job list or deletion)
					return 0;
				}	
				// The job is over  - remove job from list
				deleteJob(jobToMove->pid);
				pjobs_list->fgJob = NULL;
				return 0;
			}
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "bg")) {
		pjob_t jobToRun;
		if (num_arg > 1) {
			illegal_cmd = TRUE;
		}
		else {
			if (num_arg == 1) {
				int jobId = atoi(args[1]);
				// get the job, and verify the idx is legal
				if ((jobToRun = getJobById(jobId)) == NULL) {
					printf("smash error: > Job %d not found\n", jobId);
					illegal_cmd = TRUE;
				}
			}
				// no id was given, take the last suspended job
			else {
				if ((jobToRun = getJobByPid(pjobs_list->lastSuspendedPid)) == NULL) {
					printf("smash error: > There is no last suspended jobs to run\n");
					return 1;
				}
			}

			// Activate the selected job
			if (jobToRun != NULL){
				// Print the command
				printf("%s\n", jobToRun->command);
				// Run the job
				if (activeSuspendedJob(jobToRun) != 0) {
					perror("smash: > Send signal failed");
					return 1;
				}
				return 0;
			}
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "quit"))
	{	
		if ((num_arg > 1 || num_arg == 1) && strcmp(args[1], "kill")) {
			illegal_cmd = TRUE;
		}
		else {
			if (num_arg == 1) {
				// Iterate on the job list and send SIGTERM to all the jobs
				for(int i=0; i<pjobs_list->numOfJobsInBg ; ++i){
					bool isDeadFlag = FALSE;
					// Send sigterm and wait up to 5 sec
					pjob_t jobToKill = pjobs_list->jobs[i];

					// job to kill command name only
					char* cmdName = strtok(jobToKill->command, delimiters);
					//sendSignal(jobToKill->pid, SIGTERM);
					if (kill(jobToKill->pid, SIGTERM) == 0) {
						//signal sent succesfully
						time_t start = time(NULL);
						printf("[%d] %s - Sending SIGTERM... ", jobToKill->id, cmdName);
						while (time(NULL) - start < 5) {
							if (waitpid(jobToKill->pid, NULL, WNOHANG) != 0) {
								// job was terminated
								isDeadFlag = TRUE;
								break;
							}
						}
					}
					else {
						//failed to send signal
						perror("smash: > fail sending SIGTERM signal");
					}
					// job was not terminated, kill it!
					// If not dead after 5 sec, send sigKill
					if (!isDeadFlag){
						if (kill(jobToKill->pid, SIGKILL) == 0){
							//signal sent succesfuly
							printf("(5 sec passed) Sending SIGKILL... ");
						}
						else {
							//failed to send signal
							perror("Fail sending SIGKILL signal\n");
							exit(1);
						}
					}

					printf("Done.\n");
				}
			}
			// clean and kill smash
			deleteJobsList(pjobs_list);
			exit(0);
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "history"))
	{
		if (num_arg != 0){
			illegal_cmd = TRUE;
		}
		// Print all the history array
		else{
			for (int i=0; i<MAX_HISTORY_SIZE; ++i){
				if (strcmp(commandHistory[i], "\0")) {
					printf("%s", commandHistory[i]);	
				}
			}
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "kill"))
	{
		pjob_t destJob;
		if (num_arg != 2){
			illegal_cmd = TRUE;
		}
		else{
			int sigNum = atoi(strtok(args[1], "-"));
			int jobId = atoi(args[2]);

			// get the job
			if ((destJob = getJobById(jobId)) == NULL) {
				printf("smash error: > kill %d – job does not exist\n", jobId);
				return 1;
			}
			int jobPid = destJob->pid;

			// Send the signal to the pid
			if(sendSignal(jobPid, sigNum) != 0){
				perror("smash: > Send signal failed");
				return 1;
			}

			if(sigNum == SIGTSTP || sigNum == SIGSTOP)
			{
				destJob->is_running = FALSE;
				pjobs_list->lastSuspendedPid = jobPid;
			}

			if(sigNum == SIGCONT)
			{
				destJob->is_running = TRUE;
				// If activating the lastSuspended job, find a new suspended job
				if (pjobs_list->lastSuspendedPid == jobPid){
					pjobs_list->lastSuspendedPid = getSuspendedJob();
				}
			}

			if (sigNum == SIGKILL)
			{
				waitpid(jobPid, NULL, WNOHANG | WUNTRACED | WCONTINUED);
			}
			return 0;
		}
	}
	/*************************************************/
	else // external command
	{
 		ExeExternal(args, cmdString, 0);
	 	return 0;
	}
	if (illegal_cmd == TRUE)
	{
		printf("smash error: > \"%s\"\n", cmdString);
		return 1;
	}
    return 0;
}

///**************************************************************************************
/// function name: ExeExternal
/// Description: executes external command
/// Parameters: external command arguments, external command string
/// Returns: void
///**************************************************************************************
void ExeExternal(char *args[MAX_ARG] , char* cmdString , bool isBgCmd)
{
	int pID;
	pjob_t newJob = NULL;
    switch(pID = fork()) 
	
	{
	case -1:
		// fork did not went well throw us out
		perror("smash: > fork failure");
		exit(1);
	case 0:
		// Child Process
		// set different group id according to the instructions
		setpgrp();
		// execute the child's process
		execvp(args[0], args);
		// make sure execvp isn't returning meaning there was no error
		perror("smash: > execvp failure");
		exit(1);
	default:
		// Father process. avoid ZOMBIES by ordering the father to wait for his "still alive" son
		//create the job
		newJob = createJob(cmdString, pID);
		if (newJob == NULL) {
			printf("smash error: > cannot create external job %s\n", cmdString);
			return;
		}

		// this is an external bg job
		if (isBgCmd) {
			// add this job to the jobs list and after the fork to come the son him self will execute it
			insertJobToJobList(newJob);
		}
		else {
			// this is an external job that suppose to run in the foreground.
			// by definition we SHOULDN'T RETURN THE PROMPT TO THE USER until the job is done.
			// update FG job
			pjobs_list->fgJob = newJob;

			// wait for the son to finish running
			waitpid(pID, NULL, WUNTRACED);
			// take out the FG job from the FG "position"(it's finished) and avoid dangling ptr
			if(pjobs_list->fgJob == NULL){
				//handler took care of the job
				return;
			}
			// free the job
			free(newJob);
			pjobs_list->fgJob = NULL;
		}
	}
}


///**************************************************************************************
/// function name: ExeComp
/// Description: (don't) executes complicated command
/// Parameters: command string
/// Returns: 0- if complicated -1- if not
///**************************************************************************************
int ExeComp(char* lineSize)
{
    if ((strstr(lineSize, "|")) || (strstr(lineSize, "<")) || (strstr(lineSize, ">")) || (strstr(lineSize, "*")) || (strstr(lineSize, "?")) || (strstr(lineSize, ">>")) || (strstr(lineSize, "|&")))
    {
		return 0;
	} 
	return -1;
}


///**************************************************************************************
/// function name: BgCmd
/// Description: if command is in background, insert the command to jobs
/// Parameters: command string, pointer to jobs
/// Returns: 0- BG command -1- if not
///**************************************************************************************
int BgCmd(char* lineSize, char* cmdString)
{

	char* Command;
	char* delimiters = " \t\n";
	char *args[MAX_ARG];

	// meaning user want this command to run in the BG
	if (lineSize[strlen(lineSize)-2] == '&')
	{
		// snip the '&' char so we can execute normally
		lineSize[strlen(lineSize)-2] = '\0';
		int is_compCommand;// 0 : NOT comp  , -1 : comp command
		is_compCommand = ExeComp(lineSize);
		if (is_compCommand == 0) {
			// this is a comp command DO NOTHING(no insert to job list)
			return 0;
		}
		else {
			// this is NOT comp command execute it as external #built in commands should not called with '&'#
			int i = 0;
			Command = strtok(lineSize, delimiters);
			if (Command == NULL) {
				// no command to preform all good :)
				return 0;
			}
			args[0] = Command;
			for (i = 1; i < MAX_ARG; i++)
			{
				args[i] = strtok(NULL, delimiters);
			}
			ExeExternal(args , cmdString , TRUE);
		}
		return 0;
	}
	//this is not a BG command!
	return -1;
}


///**************************************************************************************
/// function name: activeSuspendedJob
/// Description: get valid job, send SIGCONT signal and change the lastSuspended value if needed
/// Parameters: pjob_t jobToActive
/// Returns: 0 for success,
///**************************************************************************************
int activeSuspendedJob(pjob_t jobToActive){
	// update the lastSuspended
	int ret = sendSignal(jobToActive->pid, SIGCONT);
	if(ret == 0){
		jobToActive->is_running = TRUE;
	}
	if(jobToActive->pid == pjobs_list->lastSuspendedPid){
		pjobs_list->lastSuspendedPid = getSuspendedJob();
	}
	return ret;
	
}


///**************************************************************************************
/// function name: updateHistory
/// Description: update the history array with the new command
/// Parameters: char* command
/// Returns: void
///**************************************************************************************
void updateHistory(char* command){
	int i = 0;
	// don't enter "history", "\n" and "\t" to the history array
	if (!(strcmp(command , "\t\n")) || !(strcmp(command , "\n")) || !(strcmp(command , "history\n"))){
		return;
	}

	//search for empty slot
	for (i = 0 ; i < MAX_HISTORY_SIZE; ++i) {
		if (!(strcmp(commandHistory[i] , "\0"))){			
			break;
		}
	}

	// The buffer is full, shift left all the buffer
	if (i == MAX_HISTORY_SIZE){
		i--;
		for (int j = 0; j < MAX_HISTORY_SIZE-1 ; ++j) {
			strcpy(commandHistory[j] , commandHistory[j+1]);
		}
	}
	// Copy the command to the right place
	strcpy(commandHistory[i], command);
}
//! **************************************************************************************
/// function name: initHistory
/// Description: Init the history array to "\0"
/// Parameters: none
/// Returns: void
//! **************************************************************************************
void initHistory(){
	int i = 0;	
	for (i = 0 ; i < MAX_HISTORY_SIZE; ++i) {
		strcpy(commandHistory[i] , "\0");
	}
}
