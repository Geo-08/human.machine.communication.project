#include "jobScheduler.h"

pthread_mutex_t queue_mutex ;
pthread_cond_t queue_empty ;
extern pthread_mutex_t result_mutex ;
extern pthread_cond_t resultA ;
extern uint64_t** batch_result ;

JobScheduler* Init(int num_of_threads) {
	JobScheduler* jobScheduler=malloc(sizeof(JobScheduler)) ;
	jobScheduler->num_of_threads=num_of_threads ;
	jobScheduler->threadIds=malloc(num_of_threads*sizeof(pthread_t)) ;
	jobScheduler->jobQueue=createJobQueue() ;
	pthread_mutex_init(&queue_mutex, NULL) ;
	pthread_cond_init(&queue_empty, NULL) ;
	int i ;
	for (i=0 ; i<num_of_threads ; i++) {
		pthread_create(&(jobScheduler->threadIds[i]), NULL, Run, (void*)jobScheduler) ;
	}
	return jobScheduler ;
}

int Destroy(JobScheduler* jobScheduler) {
	free(jobScheduler->threadIds) ;
	free(jobScheduler->jobQueue) ;
	free(jobScheduler) ;
}

void Barrier(JobScheduler* jobScheduler, int count) {
	int i ;
	for (i=0 ; i<count ; i++) {
		pthread_mutex_lock(&result_mutex) ;
		while(jobScheduler->executed<count) {
			pthread_cond_wait(&resultA, &result_mutex) ;
		}
		pthread_mutex_unlock(&result_mutex) ;			
	}
}

void Schedule(JobScheduler* jobScheduler, Job* job) {
	pthread_mutex_lock(&queue_mutex) ;
	addJob(jobScheduler->jobQueue, job) ;
	pthread_cond_signal(&queue_empty) ;
	pthread_mutex_unlock(&queue_mutex) ;
}

void Stop(JobScheduler* jobScheduler) {
	int i ;
	for (i=0 ; i<jobScheduler->num_of_threads ; i++) {
		pthread_kill(jobScheduler->threadIds[i], SIGUSR1) ;
		pthread_join(jobScheduler->threadIds[i], NULL) ;
	}
}

void* Run(void* data) {
	JobScheduler* jobScheduler=(JobScheduler*)data ;
	signal(SIGUSR1, thread_exit) ;
	Job* job ;
	uint64_t* result ;
	while(1) {
		pthread_mutex_lock(&queue_mutex) ;
		while(jobScheduler->jobQueue->jobCount==0) {
			pthread_cond_wait(&queue_empty, &queue_mutex) ;
		}
		job=getJob(jobScheduler->jobQueue) ;
		pthread_mutex_unlock(&queue_mutex) ;
		result=query_comp(job->tableStorage, job->query) ;
		
		pthread_mutex_lock(&result_mutex) ;
		jobScheduler->executed++ ;
		batch_result[job->index]=result ;
		QueryJobDelete(job) ;
		pthread_cond_signal(&resultA) ;
		pthread_mutex_unlock(&result_mutex) ;
	}
}

void thread_exit(int sig) {
	if (sig==SIGUSR1) {
		pthread_mutex_unlock(&queue_mutex) ;
		pthread_exit(NULL) ;
	}	
}
