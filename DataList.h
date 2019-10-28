#include <stdlib.h>
#include <inttypes.h>

#define BUCKETSIZE 65536


typedef struct Data {
	uint64_t rowIdR ;
	uint64_t rowIdS ;
} Data ;

typedef struct Bucket {
	Data data[BUCKETSIZE] ;
	int current ;
	struct Bucket* next ;
} Bucket ;

typedef struct DataList {
	Bucket* head ;
	Bucket* currentBucket ;
} DataList ;


DataList* createList(void) ;
void deleteList(DataList*) ;
void push(DataList*, Data) ;
Bucket* createBucket(void) ;
void addData(DataList*, Data) ;
