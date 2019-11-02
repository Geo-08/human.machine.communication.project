#include "sortmj.h"
#include <stdio.h>

void printresult(buffer *Buff){
	printf("\n--------RESULT--------\n");
	unsigned int used = 0;
	uint64_t row1,row2;
	while((Buff != NULL) && (used < BUFFSIZE - Buff->memleft)){
		memcpy(&row1, (Buff->memory) + used, sizeof(uint64_t));
		used += sizeof(uint64_t);
		memcpy(&row2, (Buff->memory) + used, sizeof(uint64_t));
		used += sizeof(uint64_t);
		printf("(%ld,%ld)\n", row1, row2);
		if(used + 2*sizeof(uint64_t) > BUFFSIZE){
			Buff = Buff->next;
			used = 0;
		}
	}
}

void createBuffer(buffer **Buff){
	*Buff = malloc(sizeof(buffer));
	(*Buff)->next = NULL;
	(*Buff)->memory = malloc(BUFFSIZE);
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