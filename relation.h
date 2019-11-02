#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tuple.h"

typedef struct relation {
	tuple *tuples;
	uint64_t num_tuples;
} relation;




int copy_relation (relation *to,uint64_t start,uint64_t size,relation *from);

relation* createRelation(uint64_t**,int,int);

relation* createRandomRelation(void) ;

relation* readRelation(char*) ;

void deleteRelation(relation*);


int isolate(relation *rel,uint64_t start,uint64_t size,relation *out);


void check_sorting(relation *rel,int length);

void print_relation(relation *rel, int length);
