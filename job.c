#include "job.h"

JobQueue* createJobQueue(void) {
	JobQueue* jobQueue = malloc(sizeof(JobQueue)) ;
	jobQueue->first=NULL ;
	jobQueue->last=NULL ;
	jobQueue->jobCount=0 ;
	return jobQueue ;
}

void addJob(JobQueue* jobQueue, Job* job) {
	if (jobQueue->first==NULL) {
		jobQueue->first=malloc(sizeof(Node)) ;
		jobQueue->first->job=job ;
		jobQueue->first->next=NULL ;
		jobQueue->last=jobQueue->first ;
	}
	else {
		Node* newNode=malloc(sizeof(Node)) ;
		newNode->job=job ;
		newNode->next=NULL ;
		jobQueue->last->next=newNode ;
		jobQueue->last=newNode ;
	}
	jobQueue->jobCount++ ;
}
					
Job* getJob(JobQueue* jobQueue) {
	Job* job=jobQueue->first->job ;
	Node* temp=jobQueue->first ;
	jobQueue->first=jobQueue->first->next ;
	free(temp) ;
	jobQueue->jobCount-- ;
	return job ;
}

Job* QueryJobInit(int query_index, char* query, TableStorage* tableStorage) {
	Job* job=malloc(sizeof(Job)) ;
	job->query=malloc((strlen(query)+1)*sizeof(char)) ;
	job->type=0 ;
	job->index=query_index ;
	strcpy(job->query, query) ;
	job->tableStorage=tableStorage ;
	return job ;
}

void QueryJobDelete(Job* job) {
	free(job->query) ;
	free(job) ;
}
