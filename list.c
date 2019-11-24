#include "list.h"
#include <stdio.h>


void list_init(list** l){
	*l = (list*)malloc(sizeof(list));
	(*l)->start=NULL;
	(*l)->num = 0;
}

void insert_at_end(list* l,void* new_data,size_t data_size){
	node* new_node = (node*)malloc(sizeof(node));
	new_node->data = malloc(data_size);
	new_node->next = NULL;
	int i;
	for (i=0; i<data_size; i++) 
		*(char *)(new_node->data + i) = *(char *)(new_data + i);
	if(l->num ==0)
		l->start = new_node;
	else{
		node* temp;
		temp = l->start;
		for(i=1;i<l->num;i++)
			temp = temp->next;
		temp->next = new_node;
	}
	l->num++;
}


void* return_element(list* l,int el){
	int i;
	node* temp;
	temp = l->start;
	for (i=0;i<el;i++)
		temp = temp->next;
	return temp->data;
}
