all:
	gcc  testrad.c relation.c relation.h tuple.c tuple.h sortmj.c sortmj.h -o testrad
	gcc create_test_case.c -o ctest
	gcc main.c relation.c relation.h tuple.c tuple.h sortmj.h sortmj.c -o smj
