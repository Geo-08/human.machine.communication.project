#include "sortmj.h"
#include <stdio.h>

int sort(relation *rel){
	if (rel==NULL || rel->num_tuples==0) {	//if the relation is empty or NULL 
		return 0 ;
	}
	if (rel->num_tuples*16 < MAXS){//if the relation is small on it's own jump straight into qucksort
		quicksort (rel,0,(rel->num_tuples-1));
		return 0;
	}
	relation *rel2;
	rel2 = (relation*)malloc(sizeof (relation));
	if(rel2==NULL) {
		printf("Error:Memory not allocated.") ;
		return -1 ;
	}
	rel2->tuples = (tuple*)malloc(sizeof(tuple)*rel->num_tuples);
	if(rel2->tuples==NULL) {
		printf("Error:Memory not allocated.") ;
		return -1 ;
	}
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

void printresult(buffer *Buff){
	printf("\n--------RESULT--------\n");
	unsigned int used = 0;
	uint64_t row1,row2;
	while((Buff != NULL) && (used < BUFFSIZE - Buff->memleft)){
		memcpy(&row1, (Buff->memory) + used, sizeof(uint64_t));
		used += sizeof(uint64_t);
		memcpy(&row2, (Buff->memory) + used, sizeof(uint64_t));
		used += sizeof(uint64_t);
		printf("(%lu,%lu)\n", row1, row2);
		if(used + 2*sizeof(uint64_t) > BUFFSIZE){
			Buff = Buff->next;
			used = 0;
		}
	}
}

void createBuffer(buffer **Buff){
	*Buff = malloc(sizeof(buffer));
	if((*Buff)==NULL) {
		printf("Error:Memory not allocated.") ;
		return ;
	}
	(*Buff)->next = NULL;
	(*Buff)->memory = malloc(BUFFSIZE);
	if((*Buff)->memory==NULL) {
		printf("Error:Memory not allocated.") ;
		return ;
	}
	(*Buff)->memleft = BUFFSIZE;
}

void join(uint64_t payloadR, uint64_t payloadS, buffer *Buff){
	uint64_t size;
	int used, memleft; 

	size = 2 * sizeof(uint64_t);
	while(Buff->next != NULL) 
		Buff = Buff->next;
	if(size > Buff->memleft){
		createBuffer(&(Buff->next));
		Buff = Buff->next;
	}
	used = BUFFSIZE - Buff->memleft;
	memcpy((Buff->memory) + used, &payloadR, sizeof(uint64_t));
	used += sizeof(uint64_t);
	memcpy((Buff->memory) + used, &payloadS, sizeof(uint64_t));
	Buff->memleft -= size;
}

buffer *merge(relation *relR, relation *relS){
	if (relR==NULL || relS==NULL || relR->num_tuples==0 || relS->num_tuples==0)	//if at least one relation is empty or null 
		return NULL ;
	buffer *Buff;
	tuple *tuR, *tuS;
	uint64_t counterR = 0, counterS = 0,lengthR, lengthS;
	int same = 0;

	createBuffer(&Buff);
	lengthR = relR->num_tuples;
	lengthS = relS->num_tuples;
	tuR = relR->tuples;
	tuS = relS->tuples;

	for(counterR = 0; counterR < lengthR; counterR++){	//for every tuple in relR
		if(tuR->key < tuS->key){
			tuR++;
			continue;
		}
		else if(tuR->key > tuS->key){
			while((tuR->key > tuS->key) && (counterS < lengthS)){
				counterS++;
				tuS++;
			}

		}
		//now they are equal or there are no more tuples at realtion2
		if(counterS == lengthS) break;
		while((tuR->key == tuS->key) && (counterS < lengthS)){
			join(tuR->payload, tuS->payload, Buff);//
			same++; //keeps track of the sames so the next tuple  of realtion1 if it has the same key wont miss them
			tuS++;
			counterS++;
		}
		tuR++;
		if(counterR + 1 < lengthR){
			if (tuR->key == (tuS-same)->key){ //go back if same
				tuS = tuS - same;
				counterS = counterS - same;
			}
		}
		if(counterS == lengthS) break;
		same = 0;
	}
	return Buff;
}

buffer* SortMergeJoin(relation* relR, relation* relS) {
	sort(relR);
	sort(relS);
	return merge(relR, relS) ;
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

void deletebuffer(buffer *Buff){
	buffer *nextb;
	while(Buff != NULL){
		nextb = Buff->next;
		free(Buff->memory);
		free(Buff);
		Buff = nextb;
	}
}
