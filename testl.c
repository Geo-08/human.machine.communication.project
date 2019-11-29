#include "opperations.h"
#include <stdio.h>

int main(int argc, char** argv) {
	TableStorage* store;
	store =  readTables();
	query_comp(store,"3 0 1|0.2=1.0&0.1=2.0&0.2>3499|1.2 0.1");
	/*query_comp(store,"5 0|0.2=1.0&0.3=9881|1.1 0.2 1.0");
	query_comp(store,"9 0 2|0.1=1.0&1.0=2.2&0.0>12472|1.0 0.3 0.4");
	query_comp(store,"9 0|0.1=1.0&0.1>1150|0.3 1.0");*/
	deleteTableStorage(store);
}
