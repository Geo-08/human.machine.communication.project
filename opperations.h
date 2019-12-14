#include "sortmj.h"
#include "tableStorage.h"

typedef struct inbetween{
	relation* rels;
	int num;
}inbetween;

typedef struct relation_collumn{
	int rel;
	int col;
}relation_collumn;

typedef struct filter{ 
	relation_collumn rel;
	char op;
	uint64_t num;
}filter;

typedef struct unity{
	relation_collumn rel1;
	relation_collumn rel2;
}unity;

typedef struct query{
	int* relation_numbers;
	int rnum;
	filter* filters;
	int fnum;
	unity* unitys;
	int unum;
	relation_collumn* sums;
	int snum;
}query;


void print_inb(inbetween* inb);
void equals(inbetween* inb,int place);
void join_rels(inbetween* inb,int place1,int place2);
uint64_t* query_comp(TableStorage* store,char* tq);
void lower_than (inbetween* inb,int place,uint64_t fil);
void bigger_than (inbetween* inb,int place,uint64_t fil);
void equal_to (inbetween* inb,int place,uint64_t fil);
void col_to_key2(inbetween* inb,int place,uint64_t* col,int name);
void col_to_key(inbetween* inb,int place,uint64_t* col,int name);
int find_place(inbetween* inb,int rel);
int add_relation(inbetween* inb,int name,uint64_t rows);
void inb_init(inbetween** inb);
void delete_inb(inbetween* inb);

void query_init(query** qu);
void read_query(query* qu,char* tq);
//void print_query(query* qu);
void delete_query(query* qu);
