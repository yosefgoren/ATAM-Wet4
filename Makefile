CC=gcc
COMP_FLAG= -std=c99 -g -no-pie -pedantic-errors -Wall
BINARY_PATH=bin/

all: tracer.out $(BINARY_PATH)cp1.out $(BINARY_PATH)cp2.out $(BINARY_PATH)cp3.out $(BINARY_PATH)ap1.out $(BINARY_PATH)ap2.out

$(BINARY_PATH)tracer.o: tracer.c
	$(CC) $(COMP_FLAG) -c $^ -o $@

tracer.out: $(BINARY_PATH)tracer.o
	$(CC) $(COMP_FLAG) $^ -o $@

$(BINARY_PATH)cp%.o: cp%.c
	$(CC) $(COMP_FLAG) -c $^ -o $@

$(BINARY_PATH)cp%.out: $(BINARY_PATH)cp%.o
	$(CC) $(COMP_FLAG) $^ -o $@

$(BINARY_PATH)ap%.o: ap%.asm
	as $^ -g -o $@

$(BINARY_PATH)ap%.out: $(BINARY_PATH)ap%.o
	ld $^ -o $@

clean:
	rm -f $(BINARY_PATH)*.out $(BINARY_PATH)*.o tracer.out