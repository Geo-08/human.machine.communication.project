#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "tableStorage.h"
#include "opperations.h"
#include "jobScheduler.h"
#include "job.h"

/*To run the program for small write "./script small | ./project3*/
extern pthread_mutex_t result_mutex ;
extern pthread_cond_t result_cond ;
extern Result** batch_result ;

void printResult(int query_count, JobScheduler* jobScheduler, sem_t* query_sem) {
	int i, j ;
	for (i=0 ; i<query_count ; i++) {
		for (j=0 ; j<batch_result[i]->number ; j++) {
			if(batch_result[i]->results[j]==0)
				printf("NULL ");
			else
				printf("%" PRIu64" ",batch_result[i]->results[j]);	
		}
		printf("\n");
	}
	fflush(stdout) ;
	for (i=0 ; i<query_count ; i++) {
		free(batch_result[i]->results) ;
		free(batch_result[i]) ;
	}
}

int main(int argc, char** argv) {
	int i ;
	int num_of_jobs=0 ;
	int num_of_threads=get_nprocs() ;
	int num_of_query_jobs=1 ;
	int num_of_sort_jobs=1 ;
	int num_of_join_jobs=1 ;
	for (i=1 ; i<argc ; i+=2) {
		if (strcmp(argv[i],"-t")==0)
			num_of_threads=atoi(argv[i+1]) ;
		if (strcmp(argv[i],"-q")==0)
			num_of_query_jobs=atoi(argv[i+1]) ;
		if (strcmp(argv[i],"-s")==0)
			num_of_sort_jobs=atoi(argv[i+1]) ;
		if (strcmp(argv[i],"-j")==0)
			num_of_join_jobs=atoi(argv[i+1]) ;
	}
	sem_t query_sem ;
	sem_init(&query_sem, 0, 0) ;
	struct timespec start, end ;
	double total_time ;
	int query_count=0 ;
	clock_gettime(CLOCK_MONOTONIC, &start) ;
	TableStorage* tableStorage ;
	tableStorage=readTables() ;
	size_t length=100 ;
	char* query=malloc(100*sizeof(char)) ;
	JobScheduler* jobScheduler=Init(num_of_threads, num_of_sort_jobs, num_of_join_jobs) ;
	Job* job ;
	batch_result=malloc(20*sizeof(Result*)) ;
	while (getline(&query, &length, stdin)!=-1) {
		if (strcmp(query, "F\n")!=0) {
			job=QueryJobInit(query_count, query, tableStorage, &query_sem) ;
			query_count++ ;
			Schedule(jobScheduler, job) ;
			num_of_jobs++ ;
			if (num_of_jobs==num_of_query_jobs) {
				QueryBarrier(jobScheduler, num_of_jobs, &query_sem) ;
				num_of_jobs=0 ;
			}
		}
		else {
			QueryBarrier(jobScheduler, num_of_jobs, &query_sem) ;
			num_of_jobs=0 ;
			printResult(query_count, jobScheduler, &query_sem) ;
			query_count=0 ;
		}
	}
	free(batch_result) ;
	free(query) ;
	sem_destroy(&query_sem) ;
	deleteTableStorage(tableStorage) ;
	Stop(jobScheduler) ;
	Destroy(jobScheduler) ;
	clock_gettime(CLOCK_MONOTONIC, &end) ;
	total_time=end.tv_sec-start.tv_sec ;
	total_time+=(end.tv_nsec-start.tv_nsec)/1000000000.0 ;
	printf("Total time:%f seconds.\n", total_time) ;
	return 0 ;
}
