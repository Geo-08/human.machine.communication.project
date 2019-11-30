#include "opperations.h"
#include <string.h>
#include <stdio.h>

int gcounter =0;

void query_comp(TableStorage* store,char* tq){
	printf("Query %d\n",gcounter);
	gcounter++;
	query* qu;
	query_init(&qu);
	read_query(qu,tq);
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
		if(qu->filters[i].rel.rel != inb->rels[place].keyid || colnum != inb->rels[place].keycol){
			col_to_key(inb,place,store->tables[relnum]->relations[colnum],qu->filters[i].rel.rel);//copy collumn meant to be used as key in the relation key for every tuple
			inb->rels[place].keyid = qu->filters[i].rel.rel;
			inb->rels[place].keycol = colnum;
		}
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
		if(place == -1){
			place = add_relation(inb,qu->unitys[i].rel1.rel,store->tables[relnum]->numTuples);	
		}
		if(qu->unitys[i].rel1.rel != inb->rels[place].keyid || colnum != inb->rels[place].keycol){
			col_to_key(inb,place,store->tables[relnum]->relations[colnum],qu->unitys[i].rel1.rel);
			inb->rels[place].keyid = qu->unitys[i].rel1.rel;
			inb->rels[place].keycol = colnum;
		}
		relnum2 = qu->relation_numbers[qu->unitys[i].rel2.rel];
		colnum2 = qu->unitys[i].rel2.col;
		place2 = find_place(inb,qu->unitys[i].rel2.rel);
		if(place2 == -1){
			place2 = add_relation(inb,qu->unitys[i].rel2.rel,store->tables[relnum2]->numTuples);	
		}
		if(place == place2){
			col_to_key2(inb,place2,store->tables[relnum2]->relations[colnum2],qu->unitys[i].rel2.rel);
			equals(inb,place);
			continue;
		}
		if(qu->unitys[i].rel2.rel != inb->rels[place2].keyid || colnum2 != inb->rels[place2].keycol){
			col_to_key(inb,place2,store->tables[relnum2]->relations[colnum2],qu->unitys[i].rel2.rel);
			inb->rels[place2].keyid = qu->unitys[i].rel2.rel;
			inb->rels[place2].keycol = colnum2;
		}
		if(inb->rels[place].sorted == -1){
			sort(&(inb->rels[place]));
			inb->rels[place].sorted =0;
		}
		if(inb->rels[place2].sorted == -1){
			sort(&(inb->rels[place2]));
			inb->rels[place2].sorted =0;
		}
		join_rels(inb,place,place2);//joins sorted relations
	}
	uint64_t summ,j;
	for(i=0;i<qu->snum;i++){//calculates sums
		relnum = qu->relation_numbers[qu->sums[i].rel];
		colnum = qu->sums[i].col;
		place = find_place(inb,qu->sums[i].rel);
		col_to_key(inb,place,store->tables[relnum]->relations[colnum],qu->sums[i].rel);
		summ=0;
		for(j=0;j<inb->rels[place].num_tuples;j++)
			summ = summ + inb->rels[place].tuples[j].key;
		printf("%" PRIu64 "\n",summ);		 
	}
	delete_inb(inb);
	delete_query(qu);
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
	}
	temp.keyid = inb->rels[place].keyid;
	temp.keycol = inb->rels[place].keycol;
	temp.sorted = inb->rels[place].sorted;
	for(i=0;i<inb->rels[place].num_tuples;i++)
		free(inb->rels[place].tuples[i].payload);
		free(inb->rels[place].ids);
		free(inb->rels[place].tuples);
	inb->rels[place]=temp;
}

void join_rels(inbetween* inb,int place1,int place2){
	uint64_t i,j,size,ms,z,x,flag;
	if(inb->rels[place1].num_tuples >  inb->rels[place2].num_tuples)
		size = inb->rels[place1].num_tuples;
	else
		size = inb->rels[place2].num_tuples;
	relation temp;
	temp.num_ids = inb->rels[place1].num_ids + inb->rels[place2].num_ids;
	temp.ids = (int*)malloc(sizeof(int)*(temp.num_ids));
	for(i=0;i<inb->rels[place1].num_ids;i++)
		temp.ids[i] = inb->rels[place1].ids[i];
	for(i=0;i<inb->rels[place2].num_ids;i++)
		temp.ids[i+inb->rels[place1].num_ids] = inb->rels[place2].ids[i];
	temp.tuples = (tuple*)malloc(sizeof(tuple)*size);
	temp.num_tuples = 0;
	ms=0;
	z=2;
	for(i=0;i<inb->rels[place1].num_tuples;i++){
		for(j=ms;j<inb->rels[place2].num_tuples;j++){
			if(inb->rels[place1].tuples[i].key < inb->rels[place2].tuples[j].key)
				break;
			if(inb->rels[place1].tuples[i].key > inb->rels[place2].tuples[j].key){
				ms = j+1;
				continue;	
			}
			if(inb->rels[place1].tuples[i].key == inb->rels[place2].tuples[j].key){
				if(temp.num_tuples == size*(z-1)){
					temp.tuples =(tuple*)realloc(temp.tuples,sizeof(tuple)*size*z);
					z++;
				}
				temp.tuples[temp.num_tuples].payload = (uint64_t*)malloc(sizeof(uint64_t)*temp.num_ids);
				temp.tuples[temp.num_tuples].key = inb->rels[place1].tuples[i].key;
				for(x=0;x<inb->rels[place1].num_ids;x++)
					temp.tuples[temp.num_tuples].payload[x] = inb->rels[place1].tuples[i].payload[x];
				for(x=0;x<inb->rels[place2].num_ids;x++)
					temp.tuples[temp.num_tuples].payload[x+inb->rels[place1].num_ids] = inb->rels[place2].tuples[j].payload[x];
				temp.num_tuples++;
			}
				
		}
	}
	temp.keyid = inb->rels[place1].keyid;
	temp.keycol = inb->rels[place1].keycol;
	temp.sorted = inb->rels[place1].sorted;
	for(i=0;i<inb->rels[place1].num_tuples;i++)
		free(inb->rels[place1].tuples[i].payload);
		free(inb->rels[place1].ids);
		free(inb->rels[place1].tuples);
	for(i=0;i<inb->rels[place2].num_tuples;i++)
		free(inb->rels[place2].tuples[i].payload);
		free(inb->rels[place2].ids);
		free(inb->rels[place2].tuples);
	inb->num--;
	i=0;
	flag =0;
	while(i < inb->num){
		if(flag == 1){
			inb->rels[i] = inb->rels[i+1];
		}
		if(i == place1 || i == place2){
			if(flag == 0){
				flag++;
				inb->rels[i] = temp;
			}
			else
				inb->rels[i] = inb->rels[i+1];
		}
		i++;
	}
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
			if (rel == inb->rels[i].ids[j])
				return i;
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
	int i;
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
					tempf.num = atoi(buffer);
					if(qu->fnum == 0)
						qu->filters = (filter*)malloc(sizeof(filter));
					else
						qu->filters = (filter*)realloc(qu->filters,(sizeof(filter)*(qu->fnum+1)));
					qu->filters[qu->fnum] = tempf;
					/*qu->filters[qu->fnum].op = tempf.op;
					printf("%c\n",qu->filters[qu->fnum].op);
					printf("%d,%d\n",tempf.num,qu->filters[qu->fnum].num);*/
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


void print_query(query* qu){//printing the query structure
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
}

void delete_query(query* qu){
	free(qu->relation_numbers);
	free(qu->filters);
	free(qu->unitys);
	free(qu->sums);
	free(qu);
}
