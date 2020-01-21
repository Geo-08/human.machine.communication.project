#include "opperations.h"
#include <string.h>
#include <stdio.h>

//int gcounter =0;

Result* query_comp_v2(TableStorage* store,query* qu, JobScheduler* jobScheduler){
	inbetween* inb;
	inb_init(&inb);
	int i,place;
	int relnum,colnum;
	for(i=0;i<qu->fnum;i++){//Calculate filter predicates and save them in inbetween structure 
		relnum = qu->relation_numbers[qu->filters[i].rel.rel];
		colnum = qu->filters[i].rel.col;
		place = find_place(inb,qu->filters[i].rel.rel);
		if(place == -1){//If this relation hasn't been used before add one in it
			place = add_relation(inb,qu->filters[i].rel.rel,store->tables[relnum]->numTuples);	
		}
		if(qu->filters[i].rel.rel != inb->rels[place].keyid || colnum != inb->rels[place].keycol){//Checking if the relation has used the collumn as a key before
			col_to_key(inb,place,store->tables[relnum]->relations[colnum],qu->filters[i].rel.rel);//copy collumn meant to be used as key in the relation key for every tuple
			inb->rels[place].keyid = qu->filters[i].rel.rel;
			inb->rels[place].keycol = colnum;
		}
		//Filter is executed
		if(qu->filters[i].op == '<')
			lower_than(inb,place,qu->filters[i].num);
		if(qu->filters[i].op == '>')
			bigger_than(inb,place,qu->filters[i].num);
		if(qu->filters[i].op == '=')
			equal_to(inb,place,qu->filters[i].num);
	}
	int place2;
	int relnum2,colnum2;
	for(i=0;i<qu->unum;i++){//Merge and join 
		//printf("unity %d\n",i);
		relnum = qu->relation_numbers[qu->unitys[i].rel1.rel];
		colnum = qu->unitys[i].rel1.col;
		place = find_place(inb,qu->unitys[i].rel1.rel);
		if(place == -1){//If the first relation to take part isn't in the inbetween results 
			place = add_relation(inb,qu->unitys[i].rel1.rel,store->tables[relnum]->numTuples);//add it	
		}
		if(qu->unitys[i].rel1.rel != inb->rels[place].keyid || colnum != inb->rels[place].keycol){//checking if the collumn of the relation has been used before as a key.
			col_to_key(inb,place,store->tables[relnum]->relations[colnum],qu->unitys[i].rel1.rel);//copy contents of the collumn as a key.
			inb->rels[place].keyid = qu->unitys[i].rel1.rel;
			inb->rels[place].keycol = colnum;
		}
		relnum2 = qu->relation_numbers[qu->unitys[i].rel2.rel];
		colnum2 = qu->unitys[i].rel2.col;
		place2 = find_place(inb,qu->unitys[i].rel2.rel);
		if(place2 == -1){//If the second relation to take part isn't in the inbetween results
			place2 = add_relation(inb,qu->unitys[i].rel2.rel,store->tables[relnum2]->numTuples);	
		}
		if(place == place2){//if both relations are in the same inbetween result relation, could mean they are the same.
			col_to_key2(inb,place2,store->tables[relnum2]->relations[colnum2],qu->unitys[i].rel2.rel);//copy collumn of second relation to key2.
			equals(inb,place);//do the filtering.
			continue;
		}
		if(qu->unitys[i].rel2.rel != inb->rels[place2].keyid || colnum2 != inb->rels[place2].keycol){
			col_to_key(inb,place2,store->tables[relnum2]->relations[colnum2],qu->unitys[i].rel2.rel);
			inb->rels[place2].keyid = qu->unitys[i].rel2.rel;
			inb->rels[place2].keycol = colnum2;
		}
		Job* job ;
		int count=0 ;
		int sortCount=0 ;
		sem_t sort_sem ;
		sem_init(&sort_sem, 0, 0) ;
		sem_t count_sem ;
		sem_init(&count_sem, 0, 0) ;
		pthread_mutex_t count_mutex ;
		pthread_mutex_init(&count_mutex, NULL) ;
		if(inb->rels[place].sorted == -1){//Checking if the first relation has been already sorted by it's key
			sortCount++ ;
			job=SortJobInit(&(inb->rels[place]), &sort_sem, &count_mutex, &count, &count_sem) ;
			Schedule(jobScheduler, job) ;
			inb->rels[place].sorted =0;
		}
		if(inb->rels[place2].sorted == -1){//Same for the other.
			sortCount++ ;
			job=SortJobInit(&(inb->rels[place2]), &sort_sem, &count_mutex, &count, &count_sem) ;
			Schedule(jobScheduler, job) ;
			inb->rels[place2].sorted =0;
		}
		int k ;
		for (k=0 ; k<sortCount ; k++)
			sem_wait(&count_sem) ;
		SortBarrier(jobScheduler, count, &sort_sem) ;
		pthread_mutex_destroy(&count_mutex) ;
		sem_destroy(&sort_sem) ;
		sem_destroy(&count_sem) ;
		join_rels(inb,place,place2,jobScheduler);//joins sorted relations
	}
	uint64_t summ,j;
	Result* out;
	out=malloc(sizeof(Result)) ;
	out->results = (uint64_t*)malloc(sizeof(uint64_t)*qu->snum);
	out->number=qu->snum ;
	for(i=0;i<qu->snum;i++){//calculates sums
		relnum = qu->relation_numbers[qu->sums[i].rel];
		colnum = qu->sums[i].col;
		place = find_place(inb,qu->sums[i].rel);
		col_to_key(inb,place,store->tables[relnum]->relations[colnum],qu->sums[i].rel);
		summ=0;
		for(j=0;j<inb->rels[place].num_tuples;j++)
			summ = summ + inb->rels[place].tuples[j].key;
		out->results[i] = summ;	 
	}
	delete_inb(inb);
	delete_query(qu);
	return out ;
}

uint64_t* query_comp(TableStorage* store,char* tq){
	query* qu;
	query_init(&qu);
	read_query(qu,tq);
	//optimise_query(qu) ;
	inbetween* inb;
	inb_init(&inb);
	int i,place;
	int relnum,colnum;
	for(i=0;i<qu->fnum;i++){//Calculate filter predicates and save them in inbetween structure 
		relnum = qu->relation_numbers[qu->filters[i].rel.rel];
		colnum = qu->filters[i].rel.col;
		place = find_place(inb,qu->filters[i].rel.rel);
		if(place == -1){//If this relation hasn't been used before add one in it
			place = add_relation(inb,qu->filters[i].rel.rel,store->tables[relnum]->numTuples);	
		}
		if(qu->filters[i].rel.rel != inb->rels[place].keyid || colnum != inb->rels[place].keycol){//Checking if the relation has used the collumn as a key before
			col_to_key(inb,place,store->tables[relnum]->relations[colnum],qu->filters[i].rel.rel);//copy collumn meant to be used as key in the relation key for every tuple
			inb->rels[place].keyid = qu->filters[i].rel.rel;
			inb->rels[place].keycol = colnum;
		}
		//Filter is executed
		if(qu->filters[i].op == '<')
			lower_than(inb,place,qu->filters[i].num);
		if(qu->filters[i].op == '>')
			bigger_than(inb,place,qu->filters[i].num);
		if(qu->filters[i].op == '=')
			equal_to(inb,place,qu->filters[i].num);
	}
	int place2;
	int relnum2,colnum2;
	for(i=0;i<qu->unum;i++){//Merge and join 
		//printf("unity %d\n",i);
		relnum = qu->relation_numbers[qu->unitys[i].rel1.rel];
		colnum = qu->unitys[i].rel1.col;
		place = find_place(inb,qu->unitys[i].rel1.rel);
		if(place == -1){//If the first relation to take part isn't in the inbetween results 
			place = add_relation(inb,qu->unitys[i].rel1.rel,store->tables[relnum]->numTuples);//add it	
		}
		if(qu->unitys[i].rel1.rel != inb->rels[place].keyid || colnum != inb->rels[place].keycol){//checking if the collumn of the relation has been used before as a key.
			col_to_key(inb,place,store->tables[relnum]->relations[colnum],qu->unitys[i].rel1.rel);//copy contents of the collumn as a key.
			inb->rels[place].keyid = qu->unitys[i].rel1.rel;
			inb->rels[place].keycol = colnum;
		}
		relnum2 = qu->relation_numbers[qu->unitys[i].rel2.rel];
		colnum2 = qu->unitys[i].rel2.col;
		place2 = find_place(inb,qu->unitys[i].rel2.rel);
		if(place2 == -1){//If the second relation to take part isn't in the inbetween results
			place2 = add_relation(inb,qu->unitys[i].rel2.rel,store->tables[relnum2]->numTuples);	
		}
		if(place == place2){//if both relations are in the same inbetween result relation, could mean they are the same.
			col_to_key2(inb,place2,store->tables[relnum2]->relations[colnum2],qu->unitys[i].rel2.rel);//copy collumn of second relation to key2.
			equals(inb,place);//do the filtering.
			continue;
		}
		if(qu->unitys[i].rel2.rel != inb->rels[place2].keyid || colnum2 != inb->rels[place2].keycol){
			col_to_key(inb,place2,store->tables[relnum2]->relations[colnum2],qu->unitys[i].rel2.rel);
			inb->rels[place2].keyid = qu->unitys[i].rel2.rel;
			inb->rels[place2].keycol = colnum2;
		}
		if(inb->rels[place].sorted == -1){//Checking if the first relation has been already sorted by it's key
			sort(&(inb->rels[place]));
			inb->rels[place].sorted =0;
		}
		if(inb->rels[place2].sorted == -1){//Same for the other.
			sort(&(inb->rels[place2]));
			inb->rels[place2].sorted =0;
		}
		//printf("join rels should start\n");
		//join_rels(inb,place,place2,job);//joins sorted relations
	}
	uint64_t summ,j;
	uint64_t* out;
	out = (uint64_t*)malloc(sizeof(uint64_t)*qu->snum);
	for(i=0;i<qu->snum;i++){//calculates sums
		relnum = qu->relation_numbers[qu->sums[i].rel];
		colnum = qu->sums[i].col;
		place = find_place(inb,qu->sums[i].rel);
		col_to_key(inb,place,store->tables[relnum]->relations[colnum],qu->sums[i].rel);
		summ=0;
		for(j=0;j<inb->rels[place].num_tuples;j++)
			summ = summ + inb->rels[place].tuples[j].key;
		out[i] = summ; 
	}
	delete_inb(inb);
	delete_query(qu);
	return out;
}


void equals(inbetween* inb,int place){
	uint64_t i,j;
	relation temp;
	temp.num_ids = inb->rels[place].num_ids;
	temp.ids = (int*)malloc(sizeof(int)*(temp.num_ids));
	for(i=0;i<inb->rels[place].num_ids;i++)
		temp.ids[i] = inb->rels[place].ids[i];
	temp.tuples = (tuple*)malloc(sizeof(tuple)*inb->rels[place].num_tuples);
	temp.num_tuples =0;
	for(i=0;i<inb->rels[place].num_tuples;i++){
		if(inb->rels[place].tuples[i].key == inb->rels[place].tuples[i].key2){
			temp.tuples[temp.num_tuples].payload = (uint64_t*)malloc(sizeof(uint64_t)*temp.num_ids);
			temp.tuples[temp.num_tuples].key = inb->rels[place].tuples[i].key;
			for(j=0;j<inb->rels[place].num_ids;j++)	
				temp.tuples[temp.num_tuples].payload[j] = inb->rels[place].tuples[i].payload[j];
			temp.num_tuples++;
		}
		free(inb->rels[place].tuples[i].payload);
	}
	temp.keyid = inb->rels[place].keyid;
	temp.keycol = inb->rels[place].keycol;
	temp.sorted = inb->rels[place].sorted;
	/*for(i=0;i<inb->rels[place].num_tuples;i++)
		free(inb->rels[place].tuples[i].payload);*/
		free(inb->rels[place].ids);
		free(inb->rels[place].tuples);
	inb->rels[place]=temp;
	//printf("exiting equals\n");
}
/*
void join_rels(inbetween* inb,int place1,int place2){
	uint64_t i,j,size,ts,ms,z,x,flag;
	if(inb->rels[place1].num_tuples >  inb->rels[place2].num_tuples)
		size = inb->rels[place2].num_tuples;
	else
		size = inb->rels[place1].num_tuples;
	ts = size/2;
	relation temp;
	temp.num_ids = inb->rels[place1].num_ids + inb->rels[place2].num_ids;
	temp.ids = (int*)malloc(sizeof(int)*(temp.num_ids));
	for(i=0;i<inb->rels[place1].num_ids;i++)
		temp.ids[i] = inb->rels[place1].ids[i];
	for(i=0;i<inb->rels[place2].num_ids;i++)
		temp.ids[i+inb->rels[place1].num_ids] = inb->rels[place2].ids[i];
	//temp.tuples = (tuple*)malloc(sizeof(tuple)*size);
	temp.tuples = (tuple*)malloc(1000000);
	//temp.tuples = (tuple*)malloc(sizeof(tuple));
	temp.num_tuples = 0;
	ms=0;
	z=1;
	for(i=0;i<inb->rels[place1].num_tuples;i++){
		for(j=ms;j<inb->rels[place2].num_tuples;j++){
			if(inb->rels[place1].tuples[i].key < inb->rels[place2].tuples[j].key)
				break;
			if(inb->rels[place1].tuples[i].key > inb->rels[place2].tuples[j].key){
			//	printf("num1:%" PRIu64", num2:%" PRIu64 "\n", inb->rels[place1].num_tuples, inb->rels[place2].num_tuples) ;
			//	printf("i:%" PRIu64", j:%" PRIu64 "\n", i, j) ;
				free(inb->rels[place2].tuples[j].payload);
				ms = j+1;
				continue;	
			}
			if(inb->rels[place1].tuples[i].key == inb->rels[place2].tuples[j].key){
				if((temp.num_tuples+1) * sizeof(tuple) > z*1000000){
					z++;
					temp.tuples =(tuple*)realloc(temp.tuples,1000000*z);
				}
				/*if(temp.num_tuples == size*(z-1)){
					/*size = size + ts;
					printf("%" PRIu64" ",size);
					printf("%" PRIu64" ",ts);
					temp.tuples =(tuple*)realloc(temp.tuples,sizeof(tuple)*size*z);
					z++;
				}
				/*if(temp.num_tuples == (size+((z-1)*ts)) ){
					/*size = size + ts;
					printf("%" PRIu64" ",size);
					printf("%" PRIu64" ",ts);
					//temp.tuples =(tuple*)realloc(temp.tuples,sizeof(tuple)*(size+(ts*z)) );
					if ((temp.tuples =(tuple*)realloc(temp.tuples,sizeof(tuple)*(size+(ts*z)) )) == NULL){
					   perror("socket failed");
					   exit(EXIT_FAILURE);
					}
					z++;
				}*/
				/*if(temp.num_tuples == 41665*z){
					temp.tuples =(tuple*)realloc(temp.tuples,1000000*z);
					z++;
				}*/
				/*
				//if (temp.num_tuples>0)
					//temp.tuples = (tuple*)realloc(temp.tuples,sizeof(tuple)*(temp.num_tuples+1));
				temp.tuples[temp.num_tuples].payload = (uint64_t*)malloc(sizeof(uint64_t)*temp.num_ids);
				temp.tuples[temp.num_tuples].key = inb->rels[place1].tuples[i].key;
				for(x=0;x<inb->rels[place1].num_ids;x++)
					temp.tuples[temp.num_tuples].payload[x] = inb->rels[place1].tuples[i].payload[x];
				for(x=0;x<inb->rels[place2].num_ids;x++)
					temp.tuples[temp.num_tuples].payload[x+inb->rels[place1].num_ids] = inb->rels[place2].tuples[j].payload[x];
				temp.num_tuples++;
			}
		}
		free(inb->rels[place1].tuples[i].payload);
	}
	temp.keyid = inb->rels[place1].keyid;
	temp.keycol = inb->rels[place1].keycol;
	temp.sorted = inb->rels[place1].sorted;
	/*for(i=0;i<inb->rels[place1].num_tuples;i++)
		free(inb->rels[place1].tuples[i].payload);*//*
		free(inb->rels[place1].ids);
		free(inb->rels[place1].tuples);
	for(i=ms;i<inb->rels[place2].num_tuples;i++)
		free(inb->rels[place2].tuples[i].payload);
		free(inb->rels[place2].ids);
		free(inb->rels[place2].tuples);
	inb->num--;
	i=0;
	flag =0;
	relation* temps;
	temps = (relation*)malloc(sizeof(relation)*inb->num);
	for(i=0;i<inb->num;i++){
		if(flag == 2){
			temps[i] = inb->rels[i+1];
		}
		if(flag ==0){
			if(i == place1 || i == place2){
				temps[i] = temp;
				flag++;
			}
			else
				temps[i] = inb->rels[i];
		}
		else if(flag == 1){
			if(i == place1 || i == place2){
				temps[i] = inb->rels[i+1];
				flag++;
				continue;
			}
			else
				temps[i] = inb->rels[i];
		}
	}
	free(inb->rels);
	inb->rels = temps;

}*/

//this is the pararllel join_rels
void join_rels(inbetween* inb,int place1,int place2, JobScheduler* jobScheduler){
	uint64_t i,j,size,ts,ms,z,x,flag;
	if(inb->rels[place1].num_tuples >  inb->rels[place2].num_tuples)
		size = inb->rels[place2].num_tuples;
	else
		size = inb->rels[place1].num_tuples;
	ts = size/2;
	relation temp;
	//temp.num_ids = inb->rels[place1].num_ids + inb->rels[place2].num_ids;
	temp.ids = (int*)malloc(sizeof(int)*(temp.num_ids));
	for(i=0;i<inb->rels[place1].num_ids;i++)
		temp.ids[i] = inb->rels[place1].ids[i];
	for(i=0;i<inb->rels[place2].num_ids;i++)
		temp.ids[i+inb->rels[place1].num_ids] = inb->rels[place2].ids[i];
	//temp.tuples = (tuple*)malloc(sizeof(tuple)*size);
	if (inb->rels[place1].num_tuples!=0  && inb->rels[place2].num_tuples!=0) {
		int hist_size=pow(2,BITS) ;
		uint64_t hist1[hist_size];

		create_hist(&(inb->rels[place1]),hist1,0);
		uint64_t psum1[hist_size]; 
		create_psum(hist1,psum1,0);
		uint64_t hist2[hist_size];
		create_hist(&inb->rels[place2],hist2,0);
		uint64_t psum2[hist_size]; 
		create_psum(hist2,psum2,0);
		sem_t join_sem ;
		sem_init(&join_sem, 0, 0) ;
		relation** rel1=malloc(sizeof(relation*)*jobScheduler->num_of_join_jobs) ;
		relation** rel2=malloc(sizeof(relation*)*jobScheduler->num_of_join_jobs) ;
		relation** result=malloc(sizeof(relation*)*jobScheduler->num_of_join_jobs) ;
		int chunk_size=hist_size/jobScheduler->num_of_join_jobs ;
		int remaining=hist_size-chunk_size*jobScheduler->num_of_join_jobs ;
		uint64_t start1=0, start2=0 ;
		uint64_t end1, end2 ;
		for (i=0 ; i<jobScheduler->num_of_join_jobs ; i++) {
			if (remaining>0) {
				end1=psum1[(i+1)*chunk_size]+hist1[(i+1)*chunk_size] ;
				end2=psum2[(i+1)*chunk_size]+hist2[(i+1)*chunk_size] ;
				remaining-- ;
			}
			else {
				end1=psum1[((i+1)*chunk_size)-1]+hist1[((i+1)*chunk_size)-1] ;
				end2=psum2[((i+1)*chunk_size)-1]+hist2[((i+1)*chunk_size)-1] ;
			}
			result[i]=(relation*)malloc(sizeof(relation)) ;
			rel1[i]=(relation*)malloc(sizeof(relation)) ;
	//		rel1[i]->tuples=(tuple*)malloc(inb->rels[place1].num_tuples);
			rel1[i]->tuples=&(inb->rels[place1].tuples[start1]) ;
			rel1[i]->num_tuples=end1-start1 ;
			rel1[i]->num_ids=inb->rels[place1].num_ids ;
			rel2[i]=(relation*)malloc(sizeof(relation)) ;
	//		rel2[i]->tuples=(tuple*)malloc(inb->rels[place2].num_tuples);
			rel2[i]->tuples=&(inb->rels[place2].tuples[start2]) ;
			rel2[i]->num_tuples=end2-start2 ;
			rel2[i]->num_ids= inb->rels[place2].num_ids ;
			result[i]->num_ids=rel1[i]->num_ids + rel2[i]->num_ids ;
			if (i<(jobScheduler->num_of_join_jobs-1)) {
				Job* job=JoinJobInit(result[i], rel1[i], rel2[i], &join_sem) ;
				Schedule(jobScheduler, job) ;
				start1=end1 ;
				start2=end2 ;
			}
		}
		join(result[i-1], rel1[i-1], rel2[i-1]) ;
		JoinBarrier(jobScheduler, jobScheduler->num_of_join_jobs-1, &join_sem) ;
		sem_destroy(&join_sem) ;
		uint64_t offset=0 ;
		temp.num_tuples=0 ;
		for (i=0 ; i<jobScheduler->num_of_join_jobs ; i++) {
			if (i==0)
				temp.tuples=(tuple*)malloc(1000000000) ;
			else
				temp.tuples=realloc(temp.tuples, (result[i]->num_tuples+offset)*sizeof(tuple)) ;
			temp.num_tuples+=result[i]->num_tuples ;
			memcpy(temp.tuples+offset, result[i]->tuples, result[i]->num_tuples*sizeof(tuple)) ;
			offset+=result[i]->num_tuples ;
			free(result[i]->tuples) ;
		}
		free(rel1) ;
		free(rel2) ;
		free(result) ;
	}
	
	//join(&temp, &(inb->rels[place1]), &(inb->rels[place2])) ;
	temp.keyid = inb->rels[place1].keyid;
	temp.keycol = inb->rels[place1].keycol;
	temp.sorted = inb->rels[place1].sorted;
	//for(i=0;i<inb->rels[place1].num_tuples;i++)
	//	free(inb->rels[place1].tuples[i].payload);
		free(inb->rels[place1].ids);
		free(inb->rels[place1].tuples);
	//for(i=ms;i<inb->rels[place2].num_tuples;i++)
	//	free(inb->rels[place2].tuples[i].payload);
		free(inb->rels[place2].ids);
		free(inb->rels[place2].tuples);
	inb->num--;
	i=0;
	flag =0;
	relation* temps;
	temps = (relation*)malloc(sizeof(relation)*inb->num);
	for(i=0;i<inb->num;i++){
		if(flag == 2){
			temps[i] = inb->rels[i+1];
		}
		if(flag ==0){
			if(i == place1 || i == place2){
				temps[i] = temp;
				flag++;
			}
			else
				temps[i] = inb->rels[i];
		}
		else if(flag == 1){
			if(i == place1 || i == place2){
				temps[i] = inb->rels[i+1];
				flag++;
				continue;
			}
			else
				temps[i] = inb->rels[i];
		}
	}
	free(inb->rels);
	inb->rels = temps;

}


void join(relation* result, relation* rel1, relation* rel2) {
	uint64_t i, j, ms, z, x ;
	result->tuples = (tuple*)malloc(128000);
	result->num_tuples = 0;
	ms=0;
	z=1;
	for(i=0;i<rel1->num_tuples;i++){
		for(j=ms;j<rel2->num_tuples;j++){
			if(rel1->tuples[i].key < rel2->tuples[j].key)
				break;
			if(rel1->tuples[i].key > rel2->tuples[j].key){
				free(rel2->tuples[j].payload);
				ms = j+1;
				continue;	
			}
			if(rel1->tuples[i].key == rel2->tuples[j].key){
				if((result->num_tuples+1) * sizeof(tuple) > z*128000){
					z++;
					result->tuples =(tuple*)realloc(result->tuples,128000*z);
				}
				result->tuples[result->num_tuples].payload = (uint64_t*)malloc(sizeof(uint64_t)*result->num_ids);
				result->tuples[result->num_tuples].key = rel1->tuples[i].key;
				for(x=0;x<rel1->num_ids;x++)
					result->tuples[result->num_tuples].payload[x] = rel1->tuples[i].payload[x];
				for(x=0;x<rel2->num_ids;x++)
					result->tuples[result->num_tuples].payload[x+rel1->num_ids] = rel2->tuples[j].payload[x];
				result->num_tuples++;
			}
		}
		free(rel1->tuples[i].payload);
	}
	for(i=ms;i<rel2->num_tuples;i++)
		free(rel2->tuples[i].payload);
}


void lower_than (inbetween* inb,int place,uint64_t fil){
	int i,j;
	tuple* temp;
	temp = (tuple*)malloc(sizeof(tuple)*inb->rels[place].num_tuples);
	int num =0;
	for(i=0;i<inb->rels[place].num_tuples;i++){
		if(inb->rels[place].tuples[i].key < fil){
			temp[num].key = inb->rels[place].tuples[i].key;
			temp[num].payload = (uint64_t*)malloc(sizeof(uint64_t)*inb->rels[place].num_ids);
			for(j=0;j<inb->rels[place].num_ids;j++)
				temp[num].payload[j] = inb->rels[place].tuples[i].payload[j];
			num++;
		}
		free(inb->rels[place].tuples[i].payload);
	}
	inb->rels[place].num_tuples=num;
	free(inb->rels[place].tuples);
	inb->rels[place].tuples = temp;
}

void bigger_than (inbetween* inb,int place,uint64_t fil){
	int i,j;
	tuple* temp;
	temp = (tuple*)malloc(sizeof(tuple)*inb->rels[place].num_tuples);
	int num =0;
	for(i=0;i<inb->rels[place].num_tuples;i++){
		if(inb->rels[place].tuples[i].key > fil){
			temp[num].key = inb->rels[place].tuples[i].key;
			temp[num].payload  = (uint64_t*)malloc(sizeof(uint64_t)*inb->rels[place].num_ids);
			for(j=0;j<inb->rels[place].num_ids;j++)
				temp[num].payload[j] = inb->rels[place].tuples[i].payload[j];
			num++;
		}
		free(inb->rels[place].tuples[i].payload);
	}
	inb->rels[place].num_tuples=num;
	free(inb->rels[place].tuples);
	inb->rels[place].tuples = temp;
}

void equal_to (inbetween* inb,int place,uint64_t fil){
	int i,j;
	tuple* temp;
	temp = (tuple*)malloc(sizeof(tuple)*inb->rels[place].num_tuples);
	int num =0;
	for(i=0;i<inb->rels[place].num_tuples;i++){
		if(inb->rels[place].tuples[i].key == fil){
			//temp[num].key = inb->rels[place].tuples[i].key;
			//temp[num].payload = (uint64_t*)malloc(sizeof(uint64_t)*inb->rels[place].num_ids);
			for(j=0;j<inb->rels[place].num_ids;j++)
				temp[num].payload[j] = inb->rels[place].tuples[i].payload[j];
			num++;
		}
		free(inb->rels[place].tuples[i].payload);
	}
	inb->rels[place].num_tuples=num;
	free(inb->rels[place].tuples);
	inb->rels[place].tuples = temp;
}

void col_to_key(inbetween* inb,int place,uint64_t* col,int name){
	int i,j;

	for(i=0;i<inb->rels[place].num_ids;i++){
		if(inb->rels[place].ids[i] == name)
			break;
	}
	for(j=0;j<inb->rels[place].num_tuples;j++){
		inb->rels[place].tuples[j].key = col[inb->rels[place].tuples[j].payload[i]];
	}
	inb->rels[place].sorted = -1;
}

void col_to_key2(inbetween* inb,int place,uint64_t* col,int name){
	int i,j;
	for(i=0;i<inb->rels[place].num_ids;i++){
		if(inb->rels[place].ids[i] == name)
			break;
	}
	for(j=0;j<inb->rels[place].num_tuples;j++){
		inb->rels[place].tuples[j].key2 = col[inb->rels[place].tuples[j].payload[i]];
	}
}

int find_place(inbetween* inb,int rel){
	int i;
	int j;
	for(i=0;i<inb->num;i++){
		for(j=0;j<inb->rels[i].num_ids;j++){
			if (rel == inb->rels[i].ids[j]){
				return i;
			}
		}
	}
	return -1;
}


int add_relation(inbetween* inb,int name,uint64_t rows){
	if(inb->num ==0)
		inb->rels = (relation*)malloc(sizeof(relation));
	else
		inb->rels = (relation*)realloc(inb->rels,(sizeof(relation)*(inb->num+1)));
	inb->rels[inb->num].num_ids =1;
	inb->rels[inb->num].ids = (int*)malloc(sizeof(int));
	inb->rels[inb->num].ids[0] = name;
	inb->rels[inb->num].tuples = (tuple*)malloc(sizeof(tuple)*rows);
	inb->rels[inb->num].sorted = -1;
	inb->rels[inb->num].keyid = -1;
	inb->rels[inb->num].keycol = -1;
	uint64_t i;
	for(i=0;i<rows;i++){
		inb->rels[inb->num].tuples[i].payload = (uint64_t*)malloc(sizeof(uint64_t));
		inb->rels[inb->num].tuples[i].payload[0] = i;
	}
	inb->rels[inb->num].num_tuples = rows;
	inb->num++;
	return (inb->num-1);	
}

void delete_inb(inbetween* inb){
	int i,j;
	for(i=0;i<inb->num;i++){
		for(j=0;j<inb->rels[i].num_tuples;j++)
				free(inb->rels[i].tuples[j].payload);
		
		free(inb->rels[i].tuples);
		free(inb->rels[i].ids);
	}
	free(inb->rels);
	free(inb);
}

void print_inb(inbetween* inb){
	int i,j,g;
	for(i=0;i<inb->num;i++){
		printf("Relation %d printing\n",i);
		for(j=0;j<inb->rels[i].num_ids;j++)
			printf("%d ",inb->rels[i].ids[j]);
		printf("\n");
		/*for(j=0;j<inb->rels[i].num_tuples;j++){
			printf("% " PRIu64,inb->rels[i].tuples[j].key);
			for(g=0;g<inb->rels[i].num_ids;g++)
				printf("% " PRIu64,inb->rels[i].tuples[j].payload[g]);
			printf("\n");
		}*/
	}
	
}

void inb_init(inbetween** inb){
	*inb = (inbetween*)malloc(sizeof(inbetween));
	(*inb)->num = 0;
}

void query_init(query** qu){//allocating space for query and the lists it's made up of as well as initializing them
	*qu = (query*)malloc(sizeof(query));
	(*qu)->rnum =0;
	(*qu)->fnum =0;
	(*qu)->unum =0;
	(*qu)->snum =0;
}

void read_query(query* qu,char* tq){//Parsing the string tq to save the info in our query structure qu
	int tempi;
	filter tempf;
	unity tempu;
	relation_collumn tempr;
	char buffer[20];
	int i=0,j=0,state=0,flag=0,size=strlen(tq);
	while(state == 0){//In this while the numbers of it's collumn will be inserted in the array relation_numbers with the size of ints
		if(tq[i] != ' ' && tq[i] != '|'){
			buffer[j] = tq[i];
			j++;
		}
		else{
			buffer[j] = '\0';
			if(qu->rnum == 0)
				qu->relation_numbers = (int*)malloc(sizeof(int));
			else
				qu->relation_numbers = (int*)realloc(qu->relation_numbers,(sizeof(int*)*(qu->rnum+1)));
			qu->relation_numbers[qu->rnum] = atoi(buffer);
			qu->rnum++;
			strcpy(buffer,"");
			j=0;
			if(tq[i] == '|')
				state++;
		}
		i++;
	}
	while(state == 1){//Predicates will be saved in either array filters or unitys depending on what they are 
		if(tq[i] != '.' && tq[i] != '>' && tq[i] != '<' && tq[i] != '=' && tq[i] != '&' && tq[i] != '|'){
			buffer[j] = tq[i];
			j++;
		}
		else{
			buffer[j] = '\0';
			if(tq[i] == '.'){
				tempr.rel = atoi(buffer);
				flag = 1;
			}
			if(tq[i] == '>' || tq[i] == '<' || tq[i] == '='){
				tempf.op = tq[i];
				//strcpy(&(tempf.op),&tq[i]);
				tempr.col = atoi(buffer);
				tempf.rel = tempr;
				flag =0;
			}
			if(tq[i] == '&' || tq[i] == '|'){
				if(flag == 0){//predicate wasn't a unity
					tempf.num = strtoul(buffer,NULL,10);
					if(qu->fnum == 0)
						qu->filters = (filter*)malloc(sizeof(filter));
					else
						qu->filters = (filter*)realloc(qu->filters,(sizeof(filter)*(qu->fnum+1)));
					qu->filters[qu->fnum] = tempf;
					qu->fnum++;
				}
				if(flag == 1){//means that the predicate was a unity
					tempu.rel1 = tempf.rel;
					tempr.col = atoi(buffer);
					tempu.rel2 = tempr;
					flag =0;
					if(qu->unum == 0)
						qu->unitys = (unity*)malloc(sizeof(unity));
					else
						qu->unitys = (unity*)realloc(qu->unitys,(sizeof(unity)*(qu->unum+1)));
					qu->unitys[qu->unum] = tempu;
					qu->unum++;
				}
				if(tq[i] == '|')
					state++;
			}
			strcpy(buffer,"");
			j =0;
		}
		i++;		
	}
	while(i<size){//puting the requested sums in sums array for our query structure
		if(tq[i] != '.' && tq[i] != ' '){
			buffer[j] = tq[i];
			j++;
		}
		else{
			buffer[j] = '\0';
			if(tq[i] == '.')
				tempr.rel=atoi(buffer);
			if(tq[i] == ' '){
				tempr.col = atoi(buffer);
				if(qu->snum == 0)
					qu->sums = (relation_collumn*)malloc(sizeof(relation_collumn));
				else
					qu->sums = (relation_collumn*)realloc(qu->sums,(sizeof(relation_collumn)*(qu->snum+1)));
				qu->sums[qu->snum] = tempr;
				qu->snum++;
			}
			j=0;
			strcpy(buffer,"");
		}
		i++;
	}
	buffer[j] = '\0';
	tempr.col = atoi(buffer);
	if(qu->snum == 0)
		qu->sums = (relation_collumn*)malloc(sizeof(relation_collumn));
	else
		qu->sums = (relation_collumn*)realloc(qu->sums,(sizeof(relation_collumn)*(qu->snum+1)));
	qu->sums[qu->snum] = tempr;
	qu->snum++;
}

void optimise_query(query* qu) {
	int i, j, size1=0, size2=0 ;
	int queries1[qu->unum] ;
	int queries2[qu->unum] ;
	for (i=0 ; i<qu->unum ; i++) {
		for (j=i+1 ; j<qu->unum ; j++) {
			if ((qu->unitys[i].rel1.rel==qu->unitys[j].rel1.rel && qu->unitys[i].rel1.col==qu->unitys[j].rel1.col) ||
				(qu->unitys[i].rel1.rel==qu->unitys[j].rel2.rel && qu->unitys[i].rel1.col==qu->unitys[j].rel2.col)) {
					queries1[size1]=j ;
					size1++ ;
				}
			if ((qu->unitys[i].rel2.rel==qu->unitys[j].rel1.rel && qu->unitys[i].rel2.col==qu->unitys[j].rel1.col) ||
				(qu->unitys[i].rel2.rel==qu->unitys[j].rel2.rel && qu->unitys[i].rel2.col==qu->unitys[j].rel2.col)) {
					queries2[size2]=j ;
					size2++ ;
				}	
		}
		if (size2>size1) {
			for (j=0 ; j<size2 ; j++) {
				query_swap(&(qu->unitys[i+j+1]), &(qu->unitys[queries2[j]])) ;
			}
		}
		else {
			for (j=0 ; j<size1 ; j++) {
				query_swap(&(qu->unitys[i+j+1]), &(qu->unitys[queries1[j]])) ;
			}
		}
		size1=0 ;
		size2=0 ;
	}
}

void query_swap(unity* a, unity* b) {
	unity u ;
	u.rel1 = a->rel1;
	u.rel2 = a->rel2;
	a->rel1=b->rel1 ;
	a->rel2=b->rel2 ;
	b->rel1=u.rel1 ;
	b->rel2=u.rel2 ;
}

/*void print_query(query* qu){//printing the query structure
	int i;
	printf("Printing relation numbers\n");
	for(i=0;i<qu->rnum;i++){
		printf("%d\n",qu->relation_numbers[i]);
	}
	printf("Printing filters\n");
	for(i=0;i<qu->fnum;i++){
		printf("%d.%d%c%d\n",qu->filters[i].rel.rel,qu->filters[i].rel.col,qu->filters[i].op,qu->filters[i].num);
		//printf("%c\n",qu->filters[i].op);
		//printf("%d\n",qu->filters[i].num);
	}
	printf("Printing unitys\n");
	for(i=0;i<qu->unum;i++){
		printf("%d.%d=%d.%d\n",qu->unitys[i].rel1.rel,qu->unitys[i].rel1.col,qu->unitys[i].rel2.rel,qu->unitys[i].rel2.col);
	}
	printf("Printing relations collumns\n");
	for(i=0;i<qu->snum;i++){
		printf("%d.%d\n",qu->sums[i].rel,qu->sums[i].col);
	}
}*/



void delete_query(query* qu){
	free(qu->relation_numbers);
	free(qu->filters);
	free(qu->unitys);
	free(qu->sums);
	free(qu);
}
