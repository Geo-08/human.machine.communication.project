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
	if(rel==NULL) {
		printf("Error:Memory not allocated.") ;
		return NULL ;
	}
	rel->tuples=malloc(sizeof(tuple)*size) ;
	if(rel->tuples==NULL) {
		printf("Error:Memory not allocated.") ;
		return NULL;
	}
	if (table==NULL)
		return NULL ;
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
	size=rand()%10000 ;
	relation* rel ;
	rel=malloc(sizeof(relation)) ;
	if(rel==NULL) {
		printf("Error:Memory not allocated.") ;
		return NULL;
	}
	rel->tuples=malloc(sizeof(tuple)*size) ;
	if(rel->tuples==NULL) {
		printf("Error:Memory not allocated.") ;
		return NULL;
	}
	rel->num_tuples=size ;
	for (i=0 ; i<size ; i++) {
		rel->tuples[i].key=LARGE_NUMBER +rand()%1000 ;
		rel->tuples[i].payload=LARGE_NUMBER +rand()%1000 ;
	}
	return rel ;
}

relation* readRelation(char* fileName) {
	relation* rel ;
	uint64_t key, payload ;
	FILE* file ;
	file=fopen(fileName, "r") ;
	int i=0 ;
	char* line ;
	size_t length, read ;
	char* token ;
	rel=malloc(sizeof(relation)) ;
	if(rel==NULL) {
		printf("Error:Memory not allocated.") ;
		return NULL;
	}
	if (file) {
		while((read = getline(&line, &length, file)) != -1) {
			token=strtok(line, ", ") ;
			key=strtoull(token, NULL, 10) ;
			token=strtok(NULL, ", \n") ;
			payload=strtoull(token, NULL, 10) ;
			if (i==0) {
				rel->tuples=malloc(sizeof(tuple)) ;
				if(rel->tuples==NULL) {
					printf("Error:Memory not allocated.") ;
					return NULL;
				}
			}
			else {
				rel->tuples=realloc(rel->tuples, sizeof(tuple)*(i+1)) ;
				if(rel->tuples==NULL) {
					printf("Error:Memory not allocated.") ;
					return NULL;
				}
			}
			rel->tuples[i].key=key ;
			rel->tuples[i].payload=payload ;
			i++ ;
		}
		rel->num_tuples=i ;
		fclose(file) ;
	}
	else {
		printf("Error:Can't open file.\n") ;
		return NULL ;
	}
	return rel ;
}

void deleteRelation(relation* rel) {
	free(rel->tuples) ;
	free(rel) ;
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
