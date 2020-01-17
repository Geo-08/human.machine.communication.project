#ifndef JOB_H
#define JOB_H

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "opperations.h"
#include "tableStorage.h"

typedef struct Job {
	int type ; //0:QueryJob, 1:SortJob
	int index ;
	char* query ;
	TableStorage* tableStorage ;
} Job;

typedef struct node {
	Job* job ;
	struct node* next ;
} Node;

typedef struct JobQueue {
	Node* first ;
	Node* last ;
	int jobCount ;
} JobQueue;


JobQueue* createJobQueue(void) ;
void addJob(JobQueue*, Job*) ;
Job* getJob(JobQueue*) ;

Job* QueryJobInit(int, char*, TableStorage*) ;
void QueryJobDelete(Job*) ;

#endif
