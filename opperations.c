#include "opperations.h"
#include <string.h>
#include <stdio.h>

void query_init(query** qu){//allocating space for query and the lists it's made up of as well as initializing them
	*qu = (query*)malloc(sizeof(query));
	list_init(&(*qu)->relation_numbers);
	list_init(&(*qu)->filters);
	list_init(&(*qu)->unitys);
	list_init(&(*qu)->relations_collumns);
}

void read_query(query* qu,char* tq){//Parsing the string tq to save the info in our query structure qu
	int tempi;
	filter tempf;
	unity tempu;
	relation_collumn tempr;
	char buffer[20];
	int i=0,j=0,state=0,flag=0,size=strlen(tq);
	printf("state 1\n");
	while(state == 0){//In this while the numbers of it's collumn will be inserted in the list relation_numbers with the size of ints
		if(tq[i] != ' ' && tq[i] != '|'){
			buffer[j] = tq[i];
			j++;
		}
		else{
			buffer[j] = '\0';
			tempi = atoi(buffer);
			insert_at_end(qu->relation_numbers,&tempi,sizeof(int));
			strcpy(buffer,"");
			j=0;
			if(tq[i] == '|')
				state++;
		}
		i++;
	}
	printf("state 2\n");
	while(state == 1){//Predicates will be saved in either list filters or unitys depending on what they are 
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
				tempr.col = atoi(buffer);
				tempf.rel = tempr;
				flag =0;
			}
			if(tq[i] == '&' || tq[i] == '|'){
				if(flag == 0){//predicate wasn't a unity
					tempf.num = atoi(buffer);
					insert_at_end(qu->filters,&tempf,sizeof(filter));
				}
				if(flag == 1){//means that the predicate was a unity
					tempu.rel1 = tempf.rel;
					tempr.col = atoi(buffer);
					tempu.rel2 = tempr;
					flag =0;
					insert_at_end(qu->unitys,&tempu,sizeof(unity));
				}
				if(tq[i] == '|')
					state++;
			}
			strcpy(buffer,"");
			j =0;
		}
		i++;		
	}
	printf("state 3\n");
	while(i<size){//puting the requested sums in relations_collumns list for our query structure
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
				insert_at_end(qu->relations_collumns,&tempr,sizeof(relation_collumn));
			}
			j=0;
			strcpy(buffer,"");
		}
		i++;
	}
	buffer[j] = '\0';
	tempr.col = atoi(buffer);
	insert_at_end(qu->relations_collumns,&tempr,sizeof(relation_collumn));
}


void print_query(query* qu){//printing the query structure
	int i,tempi;
	filter tempf;
	unity tempu;
	relation_collumn tempr;
	printf("Printing relation numbers\n");
	for(i=0;i<qu->relation_numbers->num;i++){
		tempi = *(int*)return_element(qu->relation_numbers,i);
		printf("%d\n",tempi);
	}
	printf("Printing filters\n");
	for(i=0;i<qu->filters->num;i++){
		tempf = *(filter*)return_element(qu->filters,i);
		printf("%d.%d%c%d\n",tempf.rel.rel,tempf.rel.col,tempf.op,tempf.num);
	}
	printf("Printing unitys\n");
	for(i=0;i<qu->unitys->num;i++){
		tempu = *(unity*)return_element(qu->unitys,i);
		printf("%d.%d=%d.%d\n",tempu.rel1.rel,tempu.rel1.col,tempu.rel2.rel,tempu.rel2.col);
	}
	printf("Printing relations collumns\n");
	for(i=0;i<qu->relations_collumns->num;i++){
		tempr = *(relation_collumn*)return_element(qu->relations_collumns,i);
		printf("%d.%d\n",tempr.rel,tempr.col);
	}
}	
