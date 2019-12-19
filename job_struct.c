#include "job_struct.h"


//! **************************************************************************************
/// function name: createJob
/// Description: create a job object with all the job fields, default id is -1
/// Parameters: command string, pid num
/// Returns: pjob_t newJob
//! **************************************************************************************
pjob_t createJob(char* command , int pid){
	pjob_t newJob = (pjob_t)malloc(sizeof(job_t));
	newJob->pid = pid;
	strcpy(newJob->command , command);
	newJob->id = -1 ;
	newJob->process_start_time = time(NULL);
	newJob->is_running = TRUE;

	return newJob;
	}

//! **************************************************************************************
/// function name: insertJobToJobList
/// Description: Insert a job to the jobs list
/// Parameters: pjob_t pJob
/// Returns: 0 for success, 1 for fail
//! **************************************************************************************
int insertJobToJobList(pjob_t pJob) {
	int numOfJobsInBg = pjobs_list->numOfJobsInBg;
	if (numOfJobsInBg == MAX_NUM_OF_JOBS){
		printf("smash error: > Job list is full - can't add the new job\n");
		return 1;
	}
	// There is room in the list, add the new job
	pjobs_list->jobs[numOfJobsInBg] = pJob;
	pjobs_list->numOfJobsInBg++;
	// assgin the job with a new unique id only if it doesn't have one
	if (pJob->id == -1){
        pJob->id = pjobs_list->jobCounter;
    }

	pjobs_list->jobCounter++;
	return 0;
}

//! **************************************************************************************
/// function name: deleteJob
/// Description: Delete a job and reorder the job list
/// Parameters: int pid (of the job to be deleted)
/// Returns: 0 for success, 1 for fail
//! **************************************************************************************
int deleteJob(int pid) {
	pjob_t jobToDelete = getJobByPid(pid);
	if (removeJobFromJobList(pid)){
		printf("smash error: > Can't delete pid %d, it doesn't exists in the job list\n", pid);
		return 1;
	}
	free(jobToDelete);
	jobToDelete = NULL;
	return 0;
}

//! **************************************************************************************
/// function name: removeJobFromJobList
/// Description: Remove a job from the jobs list and reorder the list
/// Parameters: int pid (of the job to be removed from list)
/// Returns: 0 for success, 1 for fail
//! **************************************************************************************
int removeJobFromJobList(int pid){
	int jobIdx = getJobInx(pid);
	if (jobIdx != -1){
		pjobs_list->jobs[jobIdx] = NULL;
		reorderJobList(jobIdx);
		return 0;
	}
	return 1;
}


//! **************************************************************************************
/// function name: getJobInx
/// Description: Get the index of a job in the job list
/// Parameters: int pid
/// Returns: int id of the job
//! **************************************************************************************
int getJobInx(int pid){
	pjob_t currJob = NULL;;
	for (int i = 0; i < pjobs_list->numOfJobsInBg; ++i) {
		currJob = pjobs_list->jobs[i];
		if (currJob->pid == pid)
			return i;
	}
	printf("smash error: > Can't find job with pid %d in the job list\n", pid);
	return -1;
}


//! **************************************************************************************
/// function name: getJobByPid
/// Description: Get job by pid, if not found, return NULL
/// Parameters: int pid
/// Returns: pjob_t job, or NULL if not found
//! **************************************************************************************
pjob_t getJobByPid(int pid){
	pjob_t currJob = NULL;
	for (int i = 0; i < pjobs_list->numOfJobsInBg; ++i) {
		currJob = pjobs_list->jobs[i];
		if (currJob->pid == pid)
			return currJob;
	}
	return NULL;
}


//! **************************************************************************************
/// function name: getJobById
/// Description: Get job by id, if not found, return NULL
/// Parameters: int pid
/// Returns: pjob_t job, or NULL if not found
//! **************************************************************************************
//! Get job by id, if not found, return NULL
pjob_t getJobById(int Id){
	pjob_t currJob = NULL;
	for (int i = 0; i < pjobs_list->numOfJobsInBg; ++i) {
		currJob = pjobs_list->jobs[i];
		if (currJob->id == Id)
			return currJob;
	}
	return NULL;
}


//! **************************************************************************************
/// function name: getSuspendedJob
/// Description: Get a suspended job from the end of the job list,
/// Parameters: void
/// Returns: int pid of the job. if no suspended job found return -1
//! **************************************************************************************
int getSuspendedJob(void){
	pjob_t currJob = NULL;
	for (int i = pjobs_list->numOfJobsInBg-1; i >= 0; --i) {
		currJob = pjobs_list->jobs[i];
		if (!currJob->is_running)
			return currJob->pid;
	}
	return -1;
}


//! **************************************************************************************
/// function name: deleteJobsList
/// Description: Delete all the jobs and free the jobs list at the end
/// Parameters: pjobs_list_t jobsList
/// Returns: voud
//! **************************************************************************************
void deleteJobsList(pjobs_list_t jobsList){

	if(jobsList == NULL){
		return;
	}
	for (int i=0 ; i<MAX_NUM_OF_JOBS ; ++i){
		pjob_t currJob = jobsList->jobs[i];
		if (currJob!= NULL){
			free(currJob);
		}
	}
	free(jobsList);
}


//! **************************************************************************************
/// function name: reorderJobList
/// Description: Find all the finished jobs and the deleted jobs ,and squash the list (fill all the gaps)
/// Parameters: int startingPoint, where to start the reorder from
/// Returns: void
//! **************************************************************************************
void reorderJobList(int startingPoint){
	pjob_t currJob;
	int numOfJobsInBg = pjobs_list->numOfJobsInBg;
    bool currJobIsNull = FALSE;
    int jobIsFinished = 0;
	int totalJobsRemovedFromList = 0;
	int iter = startingPoint;
	for (int i = startingPoint; i < numOfJobsInBg; ++i) {
		currJob = pjobs_list->jobs[iter];

		currJobIsNull = (currJob == NULL);
        jobIsFinished = 0;
        // if the job is not null, check if it was finished
        if (!currJobIsNull){
			jobIsFinished = waitpid(currJob->pid, NULL, WNOHANG);
		}
		if(currJobIsNull || jobIsFinished > 0 || jobIsFinished < 0){
			// Delete if the job is empty
            if(jobIsFinished>0 || jobIsFinished<0){
		if(jobIsFinished > 0){
			//job finished not by handler we responsible to delete it
			free(currJob);
		}
                currJob = NULL;				
			}
			for (int j = iter; j < numOfJobsInBg; ++j) {
				pjobs_list->jobs[j] = pjobs_list->jobs[j+1];
			}
			// Avoid skipping an element
            iter--;
			totalJobsRemovedFromList++;
		}
		iter++;
	}
	// update the job counter
	pjobs_list->numOfJobsInBg -= totalJobsRemovedFromList;
}
