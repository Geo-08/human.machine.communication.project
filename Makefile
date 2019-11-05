ALL 		= smj tests main.o SortMergeJoinTests.o CuTest.o relation.o sortmj.o tuple.o
OBJS 		= main.o relation.o sortmj.o tuple.o
TESTOBJS 	= SortMergeJoinTests.o CuTest.o relation.o sortmj.o tuple.o
OUT  		= smj
TEST  		= test

$(OUT): $(OBJS)
	gcc -o $(OUT) $(OBJS)

$(TEST): $(TESTOBJS)
	gcc -o $(TEST) $(TESTOBJS)

all: $(OUT) $(TEST)

main.o: main.c
	gcc -c main.c

relation.o: relation.c
	gcc -c relation.c

sortmj.o: sortmj.c
	gcc -c sortmj.c

tuple.o: tuple.c
	gcc -c tuple.c

SortMergeJoinTests.o: SortMergeJoinTests.c
	gcc -c SortMergeJoinTests.c

CuTest.o: CuTest.c
	gcc -c CuTest.c

clean:
	rm -f $(ALL)
