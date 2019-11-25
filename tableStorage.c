#include "tableStorage.h"

TableStorage* readTables(void) {
	char fileName[256] ;
	Table* table ;
	TableStorage* tableStorage ;
	tableStorage=malloc(sizeof(TableStorage)) ;
	tableStorage->size=0 ;
	scanf("%s", fileName) ;
	while (strcmp(fileName, "Done")!=0) {
		table=readTable(fileName) ;
		addTable(tableStorage, table) ;
		scanf("%s", fileName) ;
	}
	return tableStorage ;
}

void deleteTableStorage(TableStorage* tableStorage) {
	int i ;
	for (i=0 ; i<tableStorage->size ; i++)
		deleteTable(tableStorage->tables[i]) ;
	free(tableStorage->tables) ;
	free(tableStorage) ;
}

void addTable(TableStorage* tableStorage, Table* table) {
	if (tableStorage->size==0) {
		tableStorage->tables=malloc(sizeof(Table*)) ;
	}
	else {
		tableStorage->tables=realloc(tableStorage->tables, sizeof(Table*)*(tableStorage->size+1)) ;
	}
	tableStorage->tables[tableStorage->size]=table ;
	tableStorage->size++ ;
}

Table* createTable(uint64_t numTuples, uint64_t numColumns) {
	uint64_t i ;
	Table* table ;
	table=malloc(sizeof(Table)) ;
	table->numTuples=numTuples ;
	table->numColumns=numColumns ;
	table->relations=malloc(numColumns*sizeof(uint64_t*)) ;
	for (i=0 ; i<numColumns ; i++)
		table->relations[i]=malloc(numTuples*sizeof(uint64_t)) ;
	return table ;
}

void deleteTable(Table* table) {
	uint64_t i ;
	for (i=0 ; i<table->numColumns ; i++) {
		free(table->relations[i]) ;
	}
	free(table->relations) ;
	free(table) ;
}


Table* readTable(char* fileName) {
	char buffer[8] ;
	int i ;
	uint64_t numTuples ;
	uint64_t numColumns ;
	Table* table ;
	FILE* file ;
	file=fopen(fileName, "rb") ;
	if(file) {
		fread(&numTuples, 8, 1, file) ;
		fread(&numColumns, 8, 1, file) ;
		table=createTable(numTuples, numColumns) ;
		for (i=0 ; i<numColumns ; i++)
			fread(table->relations[i], 8, numTuples, file) ;
	}
	else {
		printf("Can't open file %s.\n", fileName) ;
		return NULL ;
	}
	fclose(file) ;
	return table ;
}
