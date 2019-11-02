#include "relation.h"







int copy_relation (relation *to,uint64_t start,uint64_t size,relation *from){ //copies contents from relation from to relation to at a specific place
	uint64_t  i;
	for(i=0;i<size;i++)
		to->tuples[i+start] = from->tuples[i];
	return 0;
}

relation* createRelation(uint64_t** table, int size, int key) {
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

relation* createRandomRelation(void) {
	int i, j ;
	uint64_t size, random_key, random_payload ;
	size=rand()%1000000 ;
	relation* rel ;
	rel=malloc(sizeof(relation)) ;
	rel->tuples=malloc(sizeof(tuple)*size) ;
	rel->num_tuples=size ;
	for (i=0 ; i<size ; i++) {
		random_key=0 ;
		random_payload=0 ;
		for (j=0 ; j<64 ; j++) {
			random_key=random_key*2 + rand()%2 ;
			random_payload=random_payload*2 + rand()%2 ;
		}
		rel->tuples[i].key =  random_key ;
		rel->tuples[i].payload =  random_payload ;
	}
	return rel ;
}

relation* readRelation(char* fileName) {
	relation* rel ;
	uint64_t key, payload ;
	FILE* file ;
	file=fopen(fileName, "r") ;
	char* line ;
	size_t length ;
	char* token ;
	rel=malloc(sizeof(relation)) ;
	if (file) {
		while(getline(&line, &length, file)) {
			token=strtok(line, ", ") ;
			key=atoi(token) ;
			printf("%s\n", token) ;
			token=strtok(NULL, ", ") ;
			payload=atoi(token) ;
		}
	}
	fclose(file) ;
	return rel ;
}

void deleteRelation(relation* rel) {
	free(rel->tuples) ;
	free(rel) ;uint64_t payS, uint64_t ;
}




int isolate(relation *rel,uint64_t start,uint64_t size,relation *out){	//takes part of rel and copies it on out
	out->num_tuples = size ;
	uint64_t i;
	for (i=0;i<size;i++)
		out->tuples[i] = rel->tuples[start+i];
	return 0;
}



void check_sorting(relation *rel,int length){  //checks if 'length' first elements of a relation are sorted, if length is 0, it sarches the whole
	if (length == 0)
		length = rel->num_tuples;
	int i;
	for(i=0;i<length-1;i++){
		if(rel->tuples[i].key >= rel->tuples[i+1].key){
			printf("not sorted!\n");
			return;
		}
	}
	printf("all sorted!\n");
	return;
}

void print_relation(relation *rel, int length){ //prints the 'legtnh' tuples of a relation, if length is 0, it prints the whole
	if (length == 0)
		length = rel->num_tuples;
	int i;
	for(i=0;i<length;i++)
		printf("%" PRIu64 " , " "%" PRIu64 "\n",rel->tuples[i].key,rel->tuples[i].payload);
}
