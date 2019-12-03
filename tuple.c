#include <stdio.h>
#include "tuple.h"

unsigned int n_first_bytes_num (tuple tup,int n){
	return ((tup.key << (BITS*n)) >> (64-BITS));
	//return ((tup.key << (4*n)) >> 60);
	//return ((tup.key << (8*n)) >> 56); // it returns a number that corresponds to the byte that is in place n of the key
	//return (tup.key >> (64-(n*8)));

}
