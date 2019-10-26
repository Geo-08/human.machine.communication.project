#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include "relation.h"

uint64_t** readArray(char* fileName, int* columns, int* rows, int *key) {
	uint64_t** array ;
	int i, j ;
	FILE* file ;
	file=fopen(fileName, "r") ;
	if (file) {
		int rows1, columns1 ;
		fscanf(file, "%d", rows) ;
		fscanf(file, "%d", columns) ;
		fscanf(file, "%d", key) ;
		array=malloc((*rows)*sizeof(uint64_t*)) ;
		for (i=0 ; i<(*rows) ; i++) {
			array[i]=malloc((*columns)*sizeof(uint64_t)) ;
		}
		for (i=0 ; i<(*rows) ; i++) {
			for (j=0 ; j<(*columns) ; j++) {
				fscanf(file, "%" SCNd64, &array[i][j]);
			}
		}
	}
	fclose(file) ;
	return array ;
}

void deleteArray(uint64_t** array, int size) {
	int i ;
	for (i=0 ; i<size ; i++) {
		free(array[i]) ;
	}
	free(array) ;
}

relation* createRelation(uint64_t** table, int size, int key) {	//this function must be moved to relation.c
	int i ;
	relation* rel ;
	rel=malloc(sizeof(relation)) ;
	rel->tuples=malloc(sizeof(tuple)*size) ;
	rel->num_tuples = size ;
	for (i=0 ; i<size ; i++) {
		rel->tuples[i].key=table[key][i] ;
		rel->tuples[i].payload=i ;
	}
	return rel ;
}

void deleteRelation(relation* rel) {	//this function must be moved to relation.c
	free(rel->tuples) ;
	free(rel) ;
}

/*result* SortMergeJoin(relation* relR, relation* relS) {
	result* res ;
	radix_sort(relR, 0) ;
	radix_sort(relS, 0) ;
	res=merge(relR, relS) ;
	return res ;
}*/

int main(int argc, char** argv) {
	if (argc<3) {
		printf("Not enough arguements.\n") ;
		return -1 ;
	}
	if (argc>3) {
		printf("Too many arguements.\n") ;
		return -1 ;
	}
	uint64_t** table1 ;
	uint64_t** table2 ;
	int i, j, key1, key2, rows1, rows2, columns1, columns2 ;
	table1=readArray(argv[1], &columns1, &rows1, &key1) ;
	table2=readArray(argv[2], &columns2, &rows2, &key2) ;
	relation* rel1 ;
	relation* rel2 ;
	rel1=createRelation(table1, columns1, key1) ;
	rel2=createRelation(table2, columns2, key2) ;
	//result=SortMergeJoin(rel1, rel2) ;
	deleteRelation(rel1) ;
	deleteRelation(rel2) ;
	deleteArray(table1, rows1) ;
	deleteArray(table2, rows2) ;
	return 0 ;
}
