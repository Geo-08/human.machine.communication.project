#ifndef JOB_H
#define JOB_H

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <semaphore.h>
#include "opperations.h"
#include "tableStorage.h"

typedef struct Job {
	int type ; //0:QueryJob, 1:SortJob, 2:JoinJob, 3:RadixSortJob
	int index ;
	char* query ;
	TableStorage* tableStorage ;
	relation* rel ;
	relation* rel2 ;
	relation* result ;
	sem_t* query_sem ;
	sem_t* sort_sem ;
	sem_t* radix_sort_sem ;
	sem_t* count_sem ;
	sem_t* join_sem ;
	pthread_mutex_t* count_mutex ;
	int* count ;
	uint64_t start ;
	uint64_t end ;
} Job;

typedef struct queue_node {
	Job* job ;
	struct queue_node* next ;
} QueueNode;

typedef struct JobQueue {
	QueueNode* first ;
	QueueNode* last ;
	int jobCount ;
} JobQueue;


JobQueue* createJobQueue(void) ;
void addJob(JobQueue*, Job*) ;
Job* getJob(JobQueue*) ;

Job* QueryJobInit(int, char*, TableStorage*, sem_t*) ;
Job* SortJobInit(relation*, sem_t*, pthread_mutex_t*, int*, sem_t*) ;
Job* RadixSortJobInit(relation*, relation*, sem_t*, uint64_t, uint64_t, sem_t*) ;
Job* JoinJobInit(relation*, relation*, relation*, sem_t*) ;
void QueryJobDelete(Job*) ;
void SortJobDelete(Job*) ;
void RadixSortJobDelete(Job*) ;
void JoinJobDelete(Job*) ;

#endif
