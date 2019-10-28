#include "DataList.h"

DataList* createList(void) {
	DataList* list ;
	list=malloc(sizeof(DataList)) ;
	list->head=NULL ;
	list->currentBucket=NULL ;
	return list ;
}

void deleteList(DataList* list) {
	Bucket* current=list->head ;
	Bucket* next ;
	while (current!=NULL) {
		next=current->next ;
		free(current) ;
		current=next ;
	}
	free(list) ;
}

void push(DataList* list, Data data) {
	if (list->head==NULL) {
		Bucket* bucket ;
		bucket=createBucket() ;
		list->head=bucket ;
		list->currentBucket=list->head ;
	}
	if (list->currentBucket->current==BUCKETSIZE) {
		Bucket* bucket ;
		bucket=createBucket() ;
		list->currentBucket->next=bucket ;
		list->currentBucket=bucket ;
	}
	addData(list, data) ;
}

Bucket* createBucket(void) {
	Bucket* bucket ;
	bucket=malloc(sizeof(Bucket)) ;
	bucket->current=0 ;
	bucket->next=NULL ;
	return bucket ;
}

void addData(DataList* list, Data data) {
	list->currentBucket->data[list->currentBucket->current]=data ;
	list->currentBucket->current++ ;
}
