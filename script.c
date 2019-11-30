#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void printFileNames(char*, char*) ;
void printQueries(char*) ;

int main(int argc, char** argv) {
	char path[30]="./" ;
	char fileName[30] ;
	char tableFile[30] ;
	char queryFile[30] ;
	strcat(path, argv[1]) ;
	strcat(path, "/") ;
	
	strcpy(fileName, path) ;
	strcat(fileName, argv[1]) ;
	strcat(fileName, ".") ;
	
	strcpy(tableFile, fileName) ;
	strcat(tableFile, "init") ;
	printFileNames(tableFile, path) ;
	strcpy(queryFile, fileName) ;
	strcat(queryFile, "work") ;
	printQueries(queryFile) ;
	return 0 ;
}


void printFileNames(char* fileName,char* path) {
	size_t length=100 ;
	FILE* file ;
	file=fopen(fileName, "r") ;
	if (file) {
		char* string=malloc(100*sizeof(char)) ;
		while (getline(&string, &length, file)!=-1)	
			printf("%s%s", path, string) ;
		printf("Done\n") ;
		free(string) ;
	}
	else {
		printf("Can't open file %s.\n", fileName) ;
		return ;
	}
	fclose(file) ;
}

void printQueries(char* fileName) {
	size_t length=100 ;
	FILE* file ;
	file=fopen(fileName, "r") ;
	if (file) {
		char* string=malloc(100*sizeof(char)) ;
		while (getline(&string, &length, file)!=-1)	
			printf("%s", string) ;
		free(string) ;
	}
	else {
		printf("Can't open file %s.\n", fileName) ;
		return ;
	}
	fclose(file) ;
}
