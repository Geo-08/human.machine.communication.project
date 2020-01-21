#ifndef RELATION_H
#define RELATION_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tuple.h"

#define LARGE_NUMBER 4294966795

typedef struct relation {
	int* ids;
	int num_ids;
	int keyid;
	int keycol;
	int sorted;
	tuple *tuples;
	uint64_t num_tuples;
	int* stprel;
} relation;




int copy_relation (relation *to,uint64_t start,uint64_t size,relation *from);

/*relation* createRelation(uint64_t**,int,int);

relation* createRandomRelation(void) ;

relation* readRelation(char*) ;*/

void deleteRelation(relation*);


int isolate(relation *rel,uint64_t start,uint64_t size,relation *out);

int isolate_v2(relation *rel,uint64_t start,uint64_t size,relation *out);

void check_sorting(relation *rel,int length);

//void print_relation(relation *rel, int length);

#endif
