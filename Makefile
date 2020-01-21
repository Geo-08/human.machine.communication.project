ALL 		= project2 script cmp main.o CuTest.o relation.o sortmj.o tuple.o opperations.o tableStorage.o opttree.o Tests.o job.o jobScheduler.o
OBJS 		= main.o relation.o sortmj.o tuple.o opperations.o tableStorage.o opttree.o job.o jobScheduler.o
TESTOBJS 	= Tests.o CuTest.o relation.o sortmj.o tuple.o opperations.o tableStorage.o opttree.o job.o jobScheduler.o
OUT  		= project3
TEST  		= test

$(OUT): $(OBJS) $(TEST) script cmp
	gcc -o $(OUT) $(OBJS) -lm -lpthread -g
	./test

$(TEST): $(TESTOBJS)
	gcc -o $(TEST) $(TESTOBJS) -lm -g -lpthread

script: script.c
	gcc -g -o script script.c
	
cmp: compareResults.c
	gcc -o cmp compareResults.c

all: $(OUT) script $(TEST)


main.o: main.c
	gcc -g -c main.c

relation.o: relation.c
	gcc -g -c relation.c

sortmj.o: sortmj.c
	gcc -g -c sortmj.c

opttree.o: opttree.c
	gcc -g -c opttree.c -lm

tuple.o: tuple.c
	gcc -g -c tuple.c

CuTest.o: CuTest.c
	gcc -g -c CuTest.c

opperations.o: opperations.c
	gcc -g -c opperations.c

tableStorage.o: tableStorage.c
	gcc -g -c tableStorage.c

job.o: job.c
	gcc -g -c job.c -lpthread
	
jobScheduler.o: jobScheduler.c
	gcc -g -c jobScheduler.c -lpthread

Tests.o: Tests.c
	gcc -g -c Tests.c


run_small: $(OUT)
	./script small | ./project3

run_small_cmp: $(OUT)
	./script small | ./project3 | ./cmp small

run_medium: $(OUT)
	./script medium | ./project3

run_medium_cmp: $(OUT)
	./script medium | ./project3 | ./cmp medium

clean:
	rm -f $(ALL)
