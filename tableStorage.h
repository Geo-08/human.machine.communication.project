#ifndef TABLESTORAGE_H
#define TABLESTORAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

typedef struct Table {
	uint64_t numTuples ;
	uint64_t numColumns ;
	uint64_t** relations ;
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

#endif
