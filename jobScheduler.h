#ifndef JOBSCHEDULER_H
#define JOBSCHEDULER_H

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "job.h"

typedef struct JobScheduler {
	int num_of_threads ;
	pthread_t* threadIds ;
	int executed ;
	JobQueue* jobQueue ;
} JobScheduler;

JobScheduler* Init(int) ;
int Destroy(JobScheduler*) ;
void Barrier(JobScheduler*, int) ;
void Schedule(JobScheduler*, Job* job) ;
void Stop(JobScheduler*) ;
void* Run(void*) ;
void thread_exit(int) ;

#endif
