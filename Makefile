CC=gcc
COMP_FLAG= -std=c99

all: tracer.out cp1.out cp2.out ap1.out

tracer.o: tracer.c
	$(CC) $(COMP_FLAG) -c $^ -o $@

tracer.out: tracer.o
	$(CC) $(COMP_FLAG) $^ -o $@

cp1.o: cp1.c
	$(CC) $(COMP_FLAG) -c $^ -o $@

cp1.out: cp1.o
	$(CC) $(COMP_FLAG) $^ -o $@


cp2.o: cp2.c
	$(CC) $(COMP_FLAG) -c $^ -o $@

cp2.out: cp2.o
	$(CC) $(COMP_FLAG) $^ -o $@

ap1.o: ap1.as
	as $^ -o $@

ap1.out: ap1.o
	ld $^ -o $@

clean:
	rm -f *.out *.o