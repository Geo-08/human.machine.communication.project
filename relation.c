#include "relation.h"

#define MAXS 65536



int sort(relation *rel){
	if (rel->num_tuples*16 < MAXS){//if the relation is small on it's own jump straight into qucksort
		quicksort (rel,0,(rel->num_tuples-1));
		return 0;
	}
	relation *rel2;
	rel2 = (relation*)malloc(sizeof (relation));
	rel2->tuples = (tuple*)malloc(sizeof(tuple)*rel->num_tuples);
	rel2->num_tuples = rel->num_tuples;
	radix_sort(rel,rel2,0,0);
	rel->num_tuples = rel2->num_tuples; 
	copy_relation(rel,0,rel2->num_tuples,rel2);
	free(rel2->tuples);
	free(rel2);
	return 0;
}

int radix_sort(relation *rel,relation *rel2, int depth,uint64_t start){	//call it with depth argument value 0, rel will point at a sorted relation at the end.
	uint64_t hist[256];
	create_hist(rel,hist,depth);
	uint64_t psum[256]; 
	create_psum(hist,psum,start);
	uint64_t i;
	unsigned int j;
	for (i=0;i<rel->num_tuples;i++){ 	//copies the contents of rel to rel2 partialy sorted
		j = n_first_bytes_num (rel->tuples[i],depth);
		rel2->tuples[psum[j]] = rel->tuples[i];
		psum[j]++;
	}
	create_psum(hist,psum,start);
	for (j=0;j<256;j++){
		if(hist[j] !=0){
			if(hist[j]*16< MAXS || depth == 8){	//checks if the amount of memory the tuples occupy in bucket j is less than 64kb
				isolate(rel2,psum[j],hist[j],rel);	//rel will have only the contents of bucket j
				quicksort (rel,0,(rel->num_tuples-1)); //rel gets sorted
				copy_relation (rel2,psum[j],hist[j],rel); //the sorted result is then copied back to rel2 in the corret place
			}
			else{ //in case the memory the tuples occupy in bucket j is more or equal to 64kb
				isolate(rel2,psum[j],hist[j],rel); //rel will have only the contents of bucket j
				radix_sort(rel,rel2,(depth+1),psum[j]); //radix_sort only on rel that now has only the contents of bucket j and depth is incr by 1
			}	
		}
	}
	return 0;
}

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
	size=rand() ;
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

void quicksort (relation *rel,uint64_t low,uint64_t high){
	if (low < high){
		uint64_t q;
		q = partition(rel,low,high);
		if(q !=0) //avoiding q-1 to give us a number larger than 0 and thus a segmentation fault
			quicksort(rel,low,q-1);
		if(q != 2^64) // avoiding q+1 to give us 0 and thus a potential segmentation fault
			quicksort(rel,q+1,high);
	}
}

uint64_t partition(relation *rel,int low,int high){
	uint64_t pivot = rel->tuples[high].key;
	uint64_t i = low;
	uint64_t j;
	for(j=low;j<high;j++){
		if(rel->tuples[j].key <= pivot){
			swap(&(rel->tuples[i]),&(rel->tuples[j]));
			i++;
		}
	}
	swap(&(rel->tuples[i]),&(rel->tuples[high]));
	return i;
}


void swap(tuple *a,tuple *b){
	tuple t;
	t = *a;
	*a = *b;
	*b = t;
}


int isolate(relation *rel,uint64_t start,uint64_t size,relation *out){	//takes part of rel and copies it on out
	out->num_tuples = size ;
	uint64_t i;
	for (i=0;i<size;i++)
		out->tuples[i] = rel->tuples[start+i];
	return 0;
}

int create_hist (relation *rel,uint64_t* hist,int n){ //creates hist
	//int hist[256];
	int i;
	for (i = 0; i <256;i++)
		hist[i] = 0;
	for (i = 0; i < rel->num_tuples; i++){
		uint64_t j = n_first_bytes_num (rel->tuples[i],n);
		hist[j]++;
	}
	return 0;
}

int create_psum (uint64_t *hist,uint64_t* psum,uint64_t start){ //creates psum
	//int psum[256];
	int i;
	psum[0] = start;
	for (i = 1; i<256;i++)
		psum[i] = psum[i-1] + hist[i-1];
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
