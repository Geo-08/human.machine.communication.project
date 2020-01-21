#include "jobScheduler.h"

pthread_mutex_t queue_mutex ;
pthread_cond_t queue_empty ;
pthread_mutex_t result_mutex ;
pthread_cond_t result_cond ;
Result** batch_result ;

JobScheduler* Init(int num_of_threads, int num_of_sort_jobs, int num_of_join_jobs) {
	JobScheduler* jobScheduler=malloc(sizeof(JobScheduler)) ;
	jobScheduler->num_of_threads=num_of_threads ;
	jobScheduler->num_of_sort_jobs=num_of_sort_jobs ;
	jobScheduler->num_of_join_jobs=num_of_join_jobs ;
	jobScheduler->threadIds=malloc(num_of_threads*sizeof(pthread_t)) ;
	jobScheduler->jobQueue=createJobQueue() ;
	pthread_mutex_init(&queue_mutex, NULL) ;
	pthread_cond_init(&queue_empty, NULL) ;
	pthread_mutex_init(&result_mutex, NULL) ;
	pthread_cond_init(&result_cond, NULL) ;
	int i ;
	for (i=0 ; i<num_of_threads ; i++) {
		pthread_create(&(jobScheduler->threadIds[i]), NULL, Run, (void*)jobScheduler) ;
	}
	return jobScheduler ;
}

int Destroy(JobScheduler* jobScheduler) {
	pthread_mutex_destroy(&queue_mutex) ;
	pthread_cond_destroy(&queue_empty) ;
	pthread_mutex_destroy(&result_mutex) ;
	pthread_cond_destroy(&result_cond) ;
	free(jobScheduler->threadIds) ;
	free(jobScheduler->jobQueue) ;
	free(jobScheduler) ;
}

void QueryBarrier(JobScheduler* jobScheduler, int count, sem_t* query_sem) {
	int i ;
	for (i=0 ; i<count ; i++) {
		sem_wait(query_sem) ;
	}
}

void SortBarrier(JobScheduler* jobScheduler, int count, sem_t* sort_sem) {
	int i ;
	for (i=0 ; i<count ; i++) {
		sem_wait(sort_sem) ;
	}
}

void JoinBarrier(JobScheduler* jobScheduler, int count, sem_t* join_sem) {
	int i ;
	for (i=0 ; i<count ; i++) {
		sem_wait(join_sem) ;
	}
}

void RadixSortBarrier(JobScheduler* jobScheduler, int count, sem_t* radix_sort_sem) {
	int i ;
	for (i=0 ; i<count ; i++) {
		sem_wait(radix_sort_sem) ;
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
	while(1) {
		pthread_mutex_lock(&queue_mutex) ;
		while(jobScheduler->jobQueue->jobCount==0) {
			pthread_cond_wait(&queue_empty, &queue_mutex) ;
		}
		job=getJob(jobScheduler->jobQueue) ;
		pthread_mutex_unlock(&queue_mutex) ;
		if (job->type==0) {
			Result* result ;
			query* qu = opt_query(job->tableStorage,job->query) ;
			result=query_comp_v2(job->tableStorage, qu, jobScheduler) ;
			pthread_mutex_lock(&result_mutex) ;
			batch_result[job->index]=result ;
			sem_post(job->query_sem) ;
			QueryJobDelete(job) ;
			pthread_mutex_unlock(&result_mutex) ;
		}
		else if (job->type==1) {
			sort_v2(job->rel, job->sort_sem, job->count_mutex, job->count, jobScheduler, job->count_sem) ;
			sem_post(job->sort_sem) ;
			SortJobDelete(job) ;
		}
		else if (job->type==2) {
			join(job->result, job->rel, job->rel2) ;
			sem_post(job->join_sem) ;
			JoinJobDelete(job) ;
		}
		else if (job->type==3) {
			radix_sort_v2(job->rel, job->rel2, 0, job->start, job->end) ;
			sem_post(job->sort_sem) ;
			sem_post(job->radix_sort_sem) ;
			RadixSortJobDelete(job) ;
		}
	}
}

void thread_exit(int sig) {
	if (sig==SIGUSR1) {
		pthread_mutex_unlock(&queue_mutex) ;
		pthread_exit(NULL) ;
	}	
}
