#include <stdio.h>
#include <string.h>
#include <stdlib.h>



int main(int argc, char** argv) {
	char fileName[30]="./" ;
	strcat(fileName, argv[1]) ;
	strcat(fileName, "/") ;
	strcat(fileName, argv[1]) ;
	strcat(fileName, ".") ;
	strcat(fileName, "result") ;
	size_t length=100 ;
	int flag=0 ;
	char* actual=malloc(100*sizeof(char)) ;
	char* expected=malloc(100*sizeof(char)) ;
	FILE* file ;
	file=fopen(fileName, "r") ;
	if (!file) {
		printf("Can't open file %s.\n", fileName) ;
		return -1 ;
	}
	while (getline(&actual, &length, stdin)!=-1) {
		getline(&expected, &length, file) ;
		expected[strlen(expected)-1]=' ' ;
		strcat(expected, "\n") ;
		if (strcmp(actual, expected)!=0) {
			flag=1 ;
			printf("Expected output %sActual output   %s\n", expected, actual) ;
		}
	}
	if (flag==0) {
		printf("All sums are correct.\n") ;
	}
	fclose(file) ;
	free(actual) ;
	free(expected) ;
	return 0 ;
}
