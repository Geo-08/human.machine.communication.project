#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include "tableStorage.h"
#include "opperations.h"
#include "opttree.h"

/*To run the program for small write "./script small | ./project2*/

int main(int argc, char** argv) {
	clock_t start, end ;
	double total_time ;
	start = clock() ;
	query* qu;
	TableStorage* tableStorage ;
	tableStorage=readTables() ;
	size_t length=100 ;
	char* query=malloc(100*sizeof(char)) ;
	while (getline(&query, &length, stdin)!=-1) {
		if (strcmp(query, "F\n")!=0) {
			//printf("start\n");
			qu = opt_query(tableStorage,query);
			//printf("end\n");
			//printf("calculating query\n");
			uint64_t* out=query_comp_v2(tableStorage, qu) ;
			//uint64_t* out=query_comp(tableStorage, query) ;
			free(out) ;
		}
	}	
	free(query) ;
	deleteTableStorage(tableStorage) ;
	end=clock() ;
	total_time=((double)(end-start))/CLOCKS_PER_SEC ;
	printf("Total time:%f seconds.\n", total_time) ;
	return 0 ;
}
