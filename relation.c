#include <stdio.h>
#include "relation.h"

#define MAXS 65536


/*int radix_sort(relation *rel,int depth){	//call it with depth argument value 0, rel will point at a sorted relation at the end.
	uint64_t hist[256];
	create_hist(rel,hist,depth);
	uint64_t psum[256]; 
	create_psum(hist,psum);
	relation *rel2;
	rel2 = (relation*) malloc(sizeof (relation));
	rel2->tuples = (tuple*)malloc(sizeof(tuple)*rel->num_tuples);
	rel2->num_tuples = rel->num_tuples;
	uint64_t i;
	unsigned int j;
	for (i=0;i<rel->num_tuples;i++){ 	//copies the contents of rel to rel2 partialy sorted
		j = n_first_bytes_num (rel->tuples[i],depth);
		rel2->tuples[psum[j]] = rel->tuples[i];
		psum[j]++;
	}
	create_psum(hist,psum);
	for (j=0;j<256;j++){
		if(hist[j] !=0){
			if(hist[j]*16< MAXS || depth == 8){	//checks if the amount of memory the tuples occupy in bucket j is less than 64kb
				isolate(rel2,psum[j],hist[j],rel);	//rel will have only the contents of bucket j
				quicksort (rel,0,(rel->num_tuples-1)); //rel gets sorted
				copy_relation (rel2,psum[j],hist[j],rel); //the sorted result is then copied back to rel2 in the corret place
			}
			else{ //in case the memory the tuples occupy in bucket j is more or equal to 64kb
				isolate(rel2,psum[j],hist[j],rel); //rel will have only the contents of bucket j
				radix_sort(rel,(depth+1)); //radix_sort only on rel that now has only the contents of bucket j and depth is incr by 1
				copy_relation (rel2,psum[j],hist[j],rel); //the sorted result is then copied back to rel2 in the correct place
			}	
		}
	}
	rel->num_tuples = rel2->num_tuples; //finally rel becomes a sorted.
	copy_relation(rel,0,rel2->num_tuples,rel2);
	free(rel2->tuples);
	free(rel2);
	return 0;
}*/

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
	printf("hi\n");
	uint64_t hist[256];
	create_hist(rel,hist,depth);
	uint64_t psum[256]; 
	create_psum(hist,psum,start);
	uint64_t i;
	unsigned int j;
	printf("gate 0\n");
	for (i=0;i<rel->num_tuples;i++){ 	//copies the contents of rel to rel2 partialy sorted
		j = n_first_bytes_num (rel->tuples[i],depth);
		rel2->tuples[psum[j]] = rel->tuples[i];
		psum[j]++;
	}
	printf("gate 1\n");
	create_psum(hist,psum,start);
	for (j=0;j<256;j++){
		if(hist[j] !=0){
			if(hist[j]*16< MAXS || depth == 8){	//checks if the amount of memory the tuples occupy in bucket j is less than 64kb
				printf("gate 2\n");
				isolate(rel2,psum[j],hist[j],rel);	//rel will have only the contents of bucket j
				printf("gate 3\n");
				quicksort (rel,0,(rel->num_tuples-1)); //rel gets sorted
				printf("gate 4\n");
				copy_relation (rel2,psum[j],hist[j],rel); //the sorted result is then copied back to rel2 in the corret place
				printf("gate 5\n");
			}
			else{ //in case the memory the tuples occupy in bucket j is more or equal to 64kb
				printf("gate 6\n");
				isolate(rel2,psum[j],hist[j],rel); //rel will have only the contents of bucket j
				printf("gate 7\n");
				radix_sort(rel,rel2,(depth+1),psum[j]); //radix_sort only on rel that now has only the contents of bucket j and depth is incr by 1
				printf("gate 8\n");
				//copy_relation (rel2,psum[j],hist[j],rel); //the sorted result is then copied back to rel2 in the correct place
				//printf("gate 9\n");
			}	
		}
	}
	/*printf("gate 10\n");
	rel->num_tuples = rel2->num_tuples; //finally rel becomes a sorted.
	printf("gate 11\n");
	copy_relation(rel,0,rel2->num_tuples,rel2);*/
	printf("all clear\n");
	return 0;
}

int copy_relation (relation *to,uint64_t start,uint64_t size,relation *from){ //copies contents from relation from to relation to at a specific place
	uint64_t  i;
	for(i=0;i<size;i++)
		to->tuples[i+start] = from->tuples[i];
	return 0;
}

void quicksort (relation *rel,uint64_t low,uint64_t high){
	if (low < high){
		uint64_t q;
		q = partition(rel,low,high);
		if(q !=0) //avoiding q-1 to give us a number larger than 0 and thus a segmentation fault
			quicksort(rel,low,q-1);
		if(q != 2^64)
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
