#include "opperations.h"
#include <stdio.h>

int main(int argc, char** argv) {
	query* qu;
	query_init(&qu);
	read_query(qu,"0 2 4|0.1=1.2&1.0=2.1&0.1>3000|0.0 1.1");
	printf("Printing Query\n");
	print_query(qu);
}
