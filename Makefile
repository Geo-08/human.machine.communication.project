all:
	gcc main.c testrad.c relation.c relation.h tuple.c tuple.h sortmj.h -o testrad
	gcc create_test_case.c -o ctest
