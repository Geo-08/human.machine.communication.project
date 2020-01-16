ALL 		= project2 script cmp main.o CuTest.o relation.o sortmj.o tuple.o opperations.o tableStorage.o opttree.o Tests.o
OBJS 		= main.o relation.o sortmj.o tuple.o opperations.o tableStorage.o opttree.o
TESTOBJS 	= Tests.o CuTest.o relation.o sortmj.o tuple.o opperations.o tableStorage.o opttree.o
OUT  		= project2
TEST  		= test

$(OUT): $(OBJS) $(TEST) script cmp
	gcc -o $(OUT) $(OBJS)
	./test

$(TEST): $(TESTOBJS)
	gcc -o $(TEST) $(TESTOBJS)

script: script.c
	gcc -o script script.c
	
cmp: compareResults.c
	gcc -o cmp compareResults.c

all: $(OUT) script $(TEST)


main.o: main.c
	gcc -c main.c

relation.o: relation.c
	gcc -c relation.c

sortmj.o: sortmj.c
	gcc -c sortmj.c

opttree.o: opttree.c
	gcc -c opttree.c -lm

tuple.o: tuple.c
	gcc -c tuple.c

CuTest.o: CuTest.c
	gcc -c CuTest.c

opperations.o: opperations.c
	gcc -c opperations.c

tableStorage.o: tableStorage.c
	gcc -c tableStorage.c

Tests.o: Tests.c
	gcc -c Tests.c


run_small: $(OUT)
	./script small | ./project2

run_small_cmp: $(OUT)
	./script small | ./project2 | ./cmp small

run_medium: $(OUT)
	./script medium | ./project2

run_medium_cmp: $(OUT)
	./script medium | ./project2 | ./cmp medium

clean:
	rm -f $(ALL)
