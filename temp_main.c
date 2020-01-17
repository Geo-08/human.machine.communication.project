#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "tableStorage.h"
#include "opperations.h"
#include "jobScheduler.h"
#include "job.h"

/*To run the program for small write "./script small | ./project2*/
pthread_mutex_t result_mutex ;
pthread_cond_t resultA ;
uint64_t** batch_result ;

void printResult(int query_count, JobScheduler* jobScheduler) {
	int i, j ;
	batch_result=malloc(query_count*sizeof(uint64_t*)) ;
	Barrier(jobScheduler, query_count) ;
	for (i=0 ; i<query_count ; i++) {
		for (j=0 ; j<1 ; j++) {
			if(batch_result[i][j]==0)
				printf("NULL ");
			else
				printf("%" PRIu64" ",batch_result[i][j]);	
		}
		printf("\n");
	}
	fflush(stdout) ;
	jobScheduler->executed=0 ;	
	for (i=0 ; i<query_count ; i++)
		free(batch_result[i]) ;
	free(batch_result) ;
}

int main(int argc, char** argv) {
	int i ;
	pthread_mutex_init(&result_mutex, NULL) ;
	pthread_cond_init(&resultA, NULL) ;
	clock_t start, end ;
	double total_time ;
	int query_count=0 ;
	start = clock() ;
	TableStorage* tableStorage ;
	tableStorage=readTables() ;
	size_t length=100 ;
	char* query=malloc(100*sizeof(char)) ;
	JobScheduler* jobScheduler=Init(2) ;
	Job* job ;
	jobScheduler->executed=0 ;	
	while (getline(&query, &length, stdin)!=-1) {
		if (strcmp(query, "F\n")!=0) {
			job=QueryJobInit(query_count, query, tableStorage) ;
			query_count++ ;
			Schedule(jobScheduler, job) ;
		}
		else {
			printResult(query_count, jobScheduler) ;
			query_count=0 ;
		}
	}
	free(query) ;
	deleteTableStorage(tableStorage) ;
	Stop(jobScheduler) ;
	Destroy(jobScheduler) ;
	end=clock() ;
	total_time=((double)(end-start))/CLOCKS_PER_SEC ;
	printf("Total time:%f seconds.\n", total_time) ;
	return 0 ;
}
