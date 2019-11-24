//#include "sortmj.h"
#include "list.h"

typedef struct relation_collumn{
	int rel;
	int col;
}relation_collumn;

typedef struct filter{ 
	relation_collumn rel;
	char op;
	int num;
}filter;

typedef struct unity{
	relation_collumn rel1;
	relation_collumn rel2;
}unity;

typedef struct query{
	list* relation_numbers;
	list* filters;
	list* unitys;
	list* relations_collumns;
}query;

void query_init(query** qu);
void read_query(query* qu,char* tq);
void print_query(query* qu);
