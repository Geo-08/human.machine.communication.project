#ifndef SORTMJ_H
#define SORTMJ_H

#include "relation.h"
#include <string.h>
#define BUFFSIZE 1024*1024
#define MAXS 65536

typedef struct buffer{
	void *memory;
	struct buffer *next;
	unsigned int memleft;
} buffer;

void createBuffer(buffer **Buff);
void join(uint64_t payloadR, uint64_t payloadS, buffer *Buff);
buffer *merge(relation *relR, relation *relS);
buffer *SortMergeJoin(relation* relR, relation* relS);
void printresult(buffer *Buff);

int sort(relation *rel);

int radix_sort(relation *rel,relation *rel2,int depth,uint64_t start);

void quicksort (relation *rel,uint64_t low,uint64_t high);

uint64_t partition(relation *rel,int low,int high);

void swap(tuple *a,tuple *b);

int create_hist (relation *rel,uint64_t* hist,int n);

int create_psum (uint64_t *hist,uint64_t* psum,uint64_t start);

#endif
