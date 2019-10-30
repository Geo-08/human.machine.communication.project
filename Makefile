all:
	gcc testrad.c relation.c relation.h tuple.c tuple.h -o testrad
	gcc create_test_case.c -o ctest
