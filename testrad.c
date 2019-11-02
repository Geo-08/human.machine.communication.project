#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "sortmj.h"

int main (int argc,char** argv){
	char in[100];
	int length = atoi(argv[1]);
	FILE* fptr;
	fptr = fopen("test_case","r");
	int i;
	relation *rel;
	rel = (relation*) malloc(sizeof(relation));
	rel->tuples = (tuple*) malloc(sizeof(tuple)*length);
	rel->num_tuples = length;
	for (i=0;i<length;i++){
		fgets(in,100,fptr);
		rel->tuples[i].key =  strtoull(strtok(in, ", \n"),NULL,10);
		rel->tuples[i].payload = strtoull(strtok(NULL, ", \n"),NULL,10);
	}
	fclose(fptr);
	//print_relation(rel,rel->num_tuples);
	//printf("first gate clear\n");
	check_sorting(rel,rel->num_tuples);
	sort(rel);
	//printf("second gate clear\n");
	//printf("%" PRIu64 "\n",rel->num_tuples);
	//print_relation(rel,rel->num_tuples);
	//printf("all gates clear\n");
	fptr = fopen("test_case_sorted", "wb+");
	check_sorting(rel,rel->num_tuples);
	for(i=0;i<length;i++)
		fprintf(fptr, "%"PRIu64", %"PRIu64"\n", rel->tuples[i].key, rel->tuples[i].payload);
	fclose(fptr);
}
