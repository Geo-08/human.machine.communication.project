#include <stdlib.h> 



typedef struct node {
	void *data;
	struct node *next;
} node;


typedef struct list {
	node *start;
	int num;
} list;



void list_init(list** l);

void insert_at_end(list* l,void* new_data,size_t data_size);


void* return_element(list* l,int el);


