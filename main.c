#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include "relation.h"
#include "sortmj.h"


uint64_t** readArray(char*, int*, int*, int*);
void deleteArray(uint64_t**, int);

int main(int argc, char** argv) {
	uint64_t** table1 ;
	uint64_t** table2 ;
	int i, j, key1, key2, rows1, rows2, columns1, columns2 ;
	relation* rel1 ;
	relation* rel2 ;
	if (argc==1) { //random
		srand(time(0)) ;
		rel1=createRandomRelation() ;
		rel2=createRandomRelation() ;
	}
	else if (argc==3) {	//read
		table1=readArray(argv[1], &columns1, &rows1, &key1) ;
		table2=readArray(argv[2], &columns2, &rows2, &key2) ;
		rel1=createRelation(table1, columns1, key1) ;
		rel2=createRelation(table2, columns2, key2) ;
	}
	else if (argc==4) {
		if (strcmp(argv[1], "dataset")) { //dataset
			//rel1=readRelation(argv[2]) ;
			//rel2=readRelation(argv[3]) ;
		}
	}
	else {
		printf("The number of arguements is incorrect.\n") ;
		return -1 ;
	}
	buffer *Buff;
	Buff=SortMergeJoin(rel1, rel2) ;
	printresult(Buff);
	deleteRelation(rel1) ;
	deleteRelation(rel2) ;
	if (argc==3) {
		deleteArray(table1, rows1) ;
		deleteArray(table2, rows2) ;
	}
	return 0 ;
}

uint64_t** readArray(char* fileName, int* columns, int* rows, int* key) {
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
