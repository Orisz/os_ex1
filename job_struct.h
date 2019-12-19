#ifndef OS_1_JOB_STRUCT_H
#define OS_1_JOB_STRUCT_H

#define MAX_LINE_SIZE 80
#define MAXARGS 20
#define MAX_NUM_OF_JOBS 100
#define MAX_HISTORY_SIZE 50
#define MAX_PATH_LENGTH 80

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum { FALSE, TRUE } bool;

typedef struct {
    int id;
    int pid;
    char command[MAX_LINE_SIZE];
    time_t process_start_time;
    bool is_running;

} job_t, *pjob_t;

typedef struct {
    pjob_t jobs[MAX_NUM_OF_JOBS];
    pjob_t fgJob;
    int numOfJobsInBg;
    int lastSuspendedPid;
    int jobCounter;
} jobs_list_t, *pjobs_list_t;

// the global jobs list struct
extern pjobs_list_t pjobs_list;
extern char lastWorkingDir[MAX_PATH_LENGTH];

/// ****************************************
//! job functions
/// ****************************************
// create a job
pjob_t createJob(char* command , int pid);

// enter job to jobList. e.g. we run new command directly in BG so we need to insert it to the jobs list
int insertJobToJobList(pjob_t pJob);

// remove job from list, delete it and reorder the list
int deleteJob(int pid);

// remove job from list and reorder the list
int removeJobFromJobList(int pid);

/// ****************************************
//!job list functions
/// ****************************************
pjob_t getJobByPid(int pid);

pjob_t getJobById(int id);

int getSuspendedJob(void);

void reorderJobList(int startingPoint);

int getJobInx(int pid);

// free all allocated memory of the jobs and the jobs list
void deleteJobsList(pjobs_list_t jobsList);

#endif //OS_1_JOB_STRUCT_H
