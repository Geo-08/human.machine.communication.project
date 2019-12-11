ALL 		= project2 script main.o CuTest.o relation.o sortmj.o tuple.o opperations.o tableStorage.o Tests.o
OBJS 		= main.o relation.o sortmj.o tuple.o opperations.o tableStorage.o
TESTOBJS 	= Tests.o CuTest.o relation.o sortmj.o tuple.o opperations.o
OUT  		= project2
TEST  		= test

$(OUT): $(OBJS) $(TEST) script
	gcc -o $(OUT) $(OBJS)
	./test

$(TEST): $(TESTOBJS)
	gcc -o $(TEST) $(TESTOBJS)

script: script.c
	gcc -o script script.c

all: $(OUT) script $(TEST)


main.o: main.c
	gcc -c main.c

relation.o: relation.c
	gcc -c relation.c

sortmj.o: sortmj.c
	gcc -c sortmj.c

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

clean:
	rm -f $(ALL)
