#ifndef TABLESTORAGE_H
#define TABLESTORAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>

#define N 50000000

typedef struct statistic{
	uint64_t l;
	uint64_t u;
	uint64_t f;
	uint64_t d;
}statistic;

typedef struct Table {
	uint64_t numTuples ;
	uint64_t numColumns ;
	uint64_t** relations ;
	statistic* stats;
} Table ;

typedef struct TableStorage {
	int size ;
	Table** tables ;	
} TableStorage ;



TableStorage* readTables(void) ;
void deleteTableStorage(TableStorage*) ;
void addTable(TableStorage*, Table*) ;
Table* createTable(uint64_t, uint64_t) ;
void deleteTable(Table*) ;
Table* readTable(char*) ;
void print_statistic(statistic stat);
void print_stats(statistic* stats,int snums);

#endif
