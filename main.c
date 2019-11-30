#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include "tableStorage.h"
#include "opperations.h"

/*To run the program for small write "./script small | ./project2*/

int main(int argc, char** argv) {
	TableStorage* tableStorage ;
	tableStorage=readTables() ;
	size_t length=100 ;
	char* query=malloc(100*sizeof(char)) ;
	while (getline(&query, &length, stdin)!=-1) {
		if (strcmp(query, "F\n")!=0)
			query_comp(tableStorage, query) ;
	}	
	free(query) ;
	deleteTableStorage(tableStorage) ;
	return 0 ;
}
