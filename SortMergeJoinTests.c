#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CuTest.h"
#include "relation.h"
#include "sortmj.h"

CuSuite* CuGetSuite() ;
void TestTwoEmptyRelation(CuTest*) ;
void TestTwoNullRelation(CuTest*) ;
void TestOneEmptyRelation(CuTest*) ;
void TestOneNullRelation(CuTest*) ;
void TestSmallRelation(CuTest*) ;
void TestSmallSameRelation(CuTest*) ;
void TestMediumNoCommonRelation(CuTest*) ;
void CuAssertBufferEquals(CuTest*, buffer*, buffer*) ;

void RunAllTests(void) {
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	CuSuiteAddSuite(suite, CuGetSuite());

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
}

int main(void) {
	RunAllTests();
}

CuSuite* CuGetSuite(void)
{
	CuSuite* suite = CuSuiteNew() ;

	SUITE_ADD_TEST(suite, TestTwoEmptyRelation) ;
	SUITE_ADD_TEST(suite, TestTwoNullRelation) ;
	SUITE_ADD_TEST(suite, TestOneEmptyRelation) ;
	SUITE_ADD_TEST(suite, TestOneNullRelation) ;
	SUITE_ADD_TEST(suite, TestSmallRelation) ;
	SUITE_ADD_TEST(suite, TestSmallSameRelation) ;
	SUITE_ADD_TEST(suite, TestMediumNoCommonRelation) ;
	
	return suite;
}

void TestTwoEmptyRelation(CuTest* tc) {
	relation* rel ;
	rel=malloc(sizeof(relation)) ;
	rel->tuples=malloc(0) ;
	rel->num_tuples=0 ;
	buffer* actual ;
	actual=SortMergeJoin(rel, rel) ;
	CuAssertPtrEquals(tc, NULL, actual) ;
	deleteRelation(rel) ;
}

void TestTwoNullRelation(CuTest* tc) {
	buffer* actual ;
	actual=SortMergeJoin(NULL, NULL) ;
	CuAssertPtrEquals(tc, NULL, actual) ;
}

void TestOneEmptyRelation(CuTest* tc) {
	relation* rel1 ;
	relation* rel2 ;
	rel1=malloc(sizeof(relation)) ;
	rel1->tuples=malloc(0) ;
	rel1->num_tuples=0 ;
	rel2=createRandomRelation() ;
	buffer* actual ;
	actual=SortMergeJoin(rel1, rel2) ;
	CuAssertPtrEquals(tc, NULL, actual) ;
	deleteRelation(rel1) ;
	deleteRelation(rel2) ;
}
void TestOneNullRelation(CuTest* tc) {
	relation* rel ;
	rel=createRandomRelation() ;
	buffer* actual ;
	actual=SortMergeJoin(rel, NULL) ;
	CuAssertPtrEquals(tc, NULL, actual) ;
	deleteRelation(rel) ;
}

void TestSmallRelation(CuTest* tc) {
	relation* rel1 ;
	relation* rel2 ;
	rel1=malloc(sizeof(relation)) ;
	rel1->tuples=malloc(sizeof(tuple)*2) ;
	rel1->num_tuples=2 ;
	rel1->tuples[0].key=1 ;
	rel1->tuples[0].payload=0 ;
	rel1->tuples[1].key=100000000000 ;
	rel1->tuples[1].payload=1 ;
	rel2=malloc(sizeof(relation)) ;
	rel2->tuples=malloc(sizeof(tuple)*2) ;
	rel2->num_tuples=2 ;
	rel2->tuples[0].key=100000000000 ;
	rel2->tuples[0].payload=0 ;
	rel2->tuples[1].key=1 ;
	rel2->tuples[1].payload=1 ;
	buffer* actual ;
	uint64_t payload1=rel1->tuples[0].payload ;
	uint64_t payload2=rel2->tuples[1].payload ;
	uint64_t payload3=rel1->tuples[1].payload ;
	uint64_t payload4=rel2->tuples[0].payload ;
	actual=SortMergeJoin(rel1, rel2) ;

	buffer* expected ;
	createBuffer(&expected) ;
	uint64_t size;
	int used, memleft; 
	size = 2 * sizeof(uint64_t);
	used = BUFFSIZE - expected->memleft;
	memcpy((expected->memory) + used, &payload1, sizeof(uint64_t));
	used += sizeof(uint64_t);
	memcpy((expected->memory) + used, &payload2, sizeof(uint64_t));
	expected->memleft -= size;
	used += sizeof(uint64_t);
	memcpy((expected->memory) + used, &payload3, sizeof(uint64_t));
	used += sizeof(uint64_t);
	memcpy((expected->memory) + used, &payload4, sizeof(uint64_t));
	expected->memleft -= size;
	CuAssertBufferEquals(tc, expected, actual) ;
	deletebuffer(actual) ;
	deletebuffer(expected) ;
	deleteRelation(rel1) ;
	deleteRelation(rel2) ;
}

void TestSmallSameRelation(CuTest* tc) {
	relation* rel ;
	rel=malloc(sizeof(relation)) ;
	rel->tuples=malloc(sizeof(tuple)*4) ;
	rel->num_tuples=4 ;
	int i ;
	for (i=0 ; i<4 ; i++) {
		rel->tuples[i].key=i ;
		rel->tuples[i].payload=i ;
	}
	buffer* actual ;
	actual=SortMergeJoin(rel, rel) ;
	
	buffer* expected ;
	createBuffer(&expected) ;
	uint64_t size;
	int used, memleft; 
	size = 2 * sizeof(uint64_t);
	used = BUFFSIZE - expected->memleft;
	uint64_t payloads[8] ;
	for (i=0 ; i<8 ; i++) {
		payloads[i]=i/2 ;
		memcpy((expected->memory) + used, &payloads[i], sizeof(uint64_t));
		used += sizeof(uint64_t);
	}
	expected->memleft -= (4*size) ;
	CuAssertBufferEquals(tc, expected, actual) ;
	deletebuffer(actual) ;
	deletebuffer(expected) ;
	deleteRelation(rel) ;
}

void TestMediumNoCommonRelation(CuTest* tc) {
	relation* rel1 ;
	relation* rel2 ;
	rel1=malloc(sizeof(relation)) ;
	rel1->tuples=malloc(sizeof(tuple)*4100) ;
	rel1->num_tuples=4100 ;
	rel2=malloc(sizeof(relation)) ;
	rel2->tuples=malloc(sizeof(tuple)*4100) ;
	rel2->num_tuples=4100 ;
	int i ;
	for (i=0 ; i<4100 ; i++) {
		rel1->tuples[i].key=i ;
		rel1->tuples[i].payload=i ;
		rel2->tuples[i].key=i+4100 ;
		rel2->tuples[i].payload=i ;
	}
	buffer* actual ;
	actual=SortMergeJoin(rel1, rel2) ;
	buffer* expected ;
	createBuffer(&expected) ;
	CuAssertBufferEquals(tc, expected, actual) ;
	deletebuffer(actual) ;
	deletebuffer(expected) ;
	deleteRelation(rel1) ;
	deleteRelation(rel2) ;
}

void CuAssertBufferEquals(CuTest* tc, buffer* expected, buffer* actual) {
	while(expected!=NULL && actual!=NULL) {
		if (expected->memleft!=actual->memleft) {
			CuFail(tc, "Different size.") ;
			return ;
		}
		if (memcmp(expected->memory, actual->memory, BUFFSIZE-expected->memleft)!=0) {
			CuFail(tc, "Different data.") ;
			return ;
		}
		expected=expected->next ;
		actual=actual->next ;
	}
	if (expected==NULL && actual==NULL)
		return ;
	CuFail(tc, "Different size.") ;
	return ;
}
