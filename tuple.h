#ifndef TUPLE_H
#define TUPLE_H

#include <inttypes.h>

typedef struct tuple {
	uint64_t key;
	uint64_t payload;
} tuple;

unsigned int n_first_bytes_num (tuple tup,int n);

#endif
