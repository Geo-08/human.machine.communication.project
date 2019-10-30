#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

int main(int argc, char ** argv)
{
	srand(time(NULL));
	if (argc!=2)
	{
		printf("Wrong Arguments\n");
		return 1;
	}
	uint64_t n, length = atoi(argv[1]), i;;

	FILE *fptr;

	fptr = fopen("test_case", "wb+");
	if(fptr == NULL)
	{
		freopen("test_case", "wb+", fptr);
	}
	for (i=0; i<length; i++)
	{
		n = ((uint64_t)rand()<<48) ^ ((uint64_t)rand()<<35) ^ ((uint64_t)rand()<<22) ^ ((uint64_t)rand()<< 9) ^ ((uint64_t)rand()>> 4);
		fprintf(fptr, "%"PRIu64", %"PRIu64"\n", n, i+1);
	}
	fclose(fptr);
	return 0;
}
