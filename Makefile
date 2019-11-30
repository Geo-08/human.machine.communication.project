ALL 		= project2 script main.o CuTest.o relation.o sortmj.o tuple.o opperations.o tableStorage.o
OBJS 		= main.o relation.o sortmj.o tuple.o opperations.o tableStorage.o
OUT  		= project2

$(OUT): $(OBJS)
	gcc -o $(OUT) $(OBJS)

script: script.c
	gcc -o script script.c

all: $(OUT) script

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

clean:
	rm -f $(ALL)
