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
		jobQueue->first=malloc(sizeof(QueueNode)) ;
		jobQueue->first->job=job ;
		jobQueue->first->next=NULL ;
		jobQueue->last=jobQueue->first ;
	}
	else {
		QueueNode* newNode=malloc(sizeof(QueueNode)) ;
		newNode->job=job ;
		newNode->next=NULL ;
		jobQueue->last->next=newNode ;
		jobQueue->last=newNode ;
	}
	jobQueue->jobCount++ ;
}
					
Job* getJob(JobQueue* jobQueue) {
	Job* job=jobQueue->first->job ;
	QueueNode* temp=jobQueue->first ;
	jobQueue->first=jobQueue->first->next ;
	free(temp) ;
	jobQueue->jobCount-- ;
	return job ;
}

Job* QueryJobInit(int query_index, char* query, TableStorage* tableStorage, sem_t* query_sem) {
	Job* job=malloc(sizeof(Job)) ;
	job->query=malloc((strlen(query)+1)*sizeof(char)) ;
	job->type=0 ;
	job->index=query_index ;
	job->query_sem=query_sem ;
	strcpy(job->query, query) ;
	job->tableStorage=tableStorage ;
	return job ;
}

void QueryJobDelete(Job* job) {
	free(job->query) ;
	free(job) ;
}

Job* SortJobInit(relation* rel, sem_t* sort_sem, pthread_mutex_t* count_mutex, int* count, sem_t* count_sem) {
	Job* job=malloc(sizeof(Job)) ;
	job->query=NULL ;
	job->tableStorage=NULL ;
	job->type=1	 ;
	job->rel=rel ;
	job->sort_sem=sort_sem ;
	job->count_sem=count_sem ;
	pthread_mutex_lock(count_mutex) ;
	job->count_mutex=count_mutex ;
	job->count=count ;
	pthread_mutex_unlock(count_mutex) ;
	return job ;
}

void SortJobDelete(Job* job) {
	free(job) ;
}

Job* RadixSortJobInit(relation* rel, relation* rel2, sem_t* sort_sem, uint64_t start, uint64_t end, sem_t* radix_sort_sem) {
	Job* job=malloc(sizeof(Job)) ;
	job->query=NULL ;
	job->tableStorage=NULL ;
	job->type=3	 ;
	job->rel=rel ;
	job->rel2=rel2 ;
	job->sort_sem=sort_sem ;
	job->radix_sort_sem=radix_sort_sem ;
	job->start=start ;
	job->end=end ;
	return job ;
}

void RadixSortJobDelete(Job* job) {
	free(job) ;
}

Job* JoinJobInit(relation* result, relation* rel, relation* rel2, sem_t* join_sem) {
	Job* job=malloc(sizeof(Job)) ;
	job->query=NULL ;
	job->tableStorage=NULL ;
	job->type=2	 ;
	job->rel=rel ;
	job->rel2=rel2 ;
	job->result=result ;
	job->join_sem=join_sem ;
	return job ;
}

void JoinJobDelete(Job* job) {
	free(job) ;
}
