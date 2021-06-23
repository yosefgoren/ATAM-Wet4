CC=gcc
COMP_FLAG= -std=c99 -g -no-pie
BINARY_PATH=bin/

all: tracer.out cp1.out cp2.out cp3.out ap1.out

tracer.o: tracer.c
	$(CC) $(COMP_FLAG) -c $^ -o $(BINARY_PATH)$@

tracer.out: tracer.o
	$(CC) $(COMP_FLAG) $(BINARY_PATH)$^ -o $@

cp1.o: cp1.c
	$(CC) $(COMP_FLAG) -c $^ -o $(BINARY_PATH)$@

cp1.out: cp1.o
	$(CC) $(COMP_FLAG) $(BINARY_PATH)$^ -o $(BINARY_PATH)$@


cp2.o: cp2.c
	$(CC) $(COMP_FLAG) -c $^ -o $(BINARY_PATH)$@

cp2.out: cp2.o
	$(CC) $(COMP_FLAG) $(BINARY_PATH)$^ -o $(BINARY_PATH)$@

cp3.o: cp3.c
	$(CC) $(COMP_FLAG) -c $^ -o $(BINARY_PATH)$@

cp3.out: cp3.o
	$(CC) $(COMP_FLAG) $(BINARY_PATH)$^ -o $(BINARY_PATH)$@

ap1.o: ap1.as
	as $^ -o $(BINARY_PATH)$@

ap1.out: ap1.o
	ld $(BINARY_PATH)$^ -o $(BINARY_PATH)$@

clean:
	rm -f $(BINARY_PATH)*.out $(BINARY_PATH)*.o tracer.out