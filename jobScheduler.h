#ifndef JOBSCHEDULER_H
#define JOBSCHEDULER_H

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include "job.h"
#include "opperations.h"
#include "opttree.h"

typedef struct JobQueue JobQueue ;
typedef struct Job Job ;

typedef struct JobScheduler {
	int num_of_threads ;
	pthread_t* threadIds ;
	int num_of_sort_jobs ;
	int num_of_join_jobs ;
	JobQueue* jobQueue ;
} JobScheduler;

typedef struct Result {
	uint64_t* results ;
	int number ;
} Result;

JobScheduler* Init(int, int, int) ;
int Destroy(JobScheduler*) ;
void QueryBarrier(JobScheduler*, int, sem_t*) ;
void SortBarrier(JobScheduler*, int, sem_t*) ;
void RadixSortBarrier(JobScheduler*, int, sem_t*) ;
void JoinBarrier(JobScheduler*, int, sem_t*) ;
void Schedule(JobScheduler*, Job* job) ;
void Stop(JobScheduler*) ;
void* Run(void*) ;
void thread_exit(int) ;

#endif
