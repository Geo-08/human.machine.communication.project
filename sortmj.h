#ifndef SORTMJ_H
#define SORTMJ_H

#include "relation.h"
#include <string.h>
#include <semaphore.h>
#include "jobScheduler.h"
#include "job.h"
#define BUFFSIZE 1024*1024
#define MAXS 65536

typedef struct JobScheduler JobScheduler ;

/*typedef struct buffer{
	void *memory;
	struct buffer *next;
	unsigned int memleft;
} buffer;

void createBuffer(buffer **Buff);
void join(uint64_t payloadR, uint64_t payloadS, buffer **Buff);
buffer *merge(relation *relR, relation *relS);
buffer *SortMergeJoin(relation* relR, relation* relS);
void printresult(buffer *Buff);
void deletebuffer(buffer *Buff);
*/

int sort(relation *rel, sem_t* sort_sem, pthread_mutex_t* count_mutex, int* count, JobScheduler* jobScheduler, sem_t* count_sem);

int radix_sort(relation *rel,relation *rel2,int depth,uint64_t start);

int radix_sort_v2(relation *rel,relation *rel2,int depth,uint64_t start,uint64_t end);

void quicksort (relation *rel,uint64_t low,uint64_t high);

uint64_t partition(relation *rel,uint64_t low,uint64_t high);

void swap(tuple *a,tuple *b);

int create_hist (relation *rel,uint64_t* hist,int n);

int create_hist_v2 (relation *rel,uint64_t* hist,int n,uint64_t start,uint64_t end);

int create_psum (uint64_t *hist,uint64_t* psum,uint64_t start);


#endif
