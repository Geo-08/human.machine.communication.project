#ifndef SORTMJ_H
#define SORTMJ_H

#include "relation.h"
#include <string.h>
#define BUFFSIZE 1024*1024

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

#endif
