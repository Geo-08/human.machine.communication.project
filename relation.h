#include <stdlib.h>
#include "tuple.h"

typedef struct relation {
	tuple *tuples;
	uint64_t num_tuples;
} relation;

typedef struct buffer{
	void *memory;
	struct buffer *next;
	unsigned int memleft;
} buffer;

int radix_sort(relation *rel,int depth);

int copy_relation (relation *to,uint64_t start,uint64_t size,relation *from);

void quicksort (relation *rel,uint64_t low,uint64_t high);

uint64_t partition(relation *rel,int low,int high);

void swap(tuple *a,tuple *b);

int isolate(relation *rel,uint64_t start,uint64_t size,relation *out);

int create_hist (relation *rel,uint64_t* hist,int n);

int create_psum (uint64_t *hist,uint64_t* psum);

void check_sorting(relation *rel,int length);

void print_relation(relation *rel, int length);
//int print_hist (int* hist);
