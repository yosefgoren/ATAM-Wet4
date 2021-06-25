CC=gcc
COMP_FLAG= -std=c99 -g -no-pie -pedantic-errors -Wall
BINARY_PATH=bin/

all: prf.out $(BINARY_PATH)cp1.out $(BINARY_PATH)cp2.out $(BINARY_PATH)cp3.out $(BINARY_PATH)ap1.out $(BINARY_PATH)ap2.out copyToTests

copyToTests: prf.c prf.out
	cp prf.c a_test/prf.c
	cp prf.out b_test/prf.out
	cp prf.out c_test/prf.out

$(BINARY_PATH)prf.o: prf.c
	$(CC) $(COMP_FLAG) -c $^ -o $@

prf.out: $(BINARY_PATH)prf.o
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
	rm -f $(BINARY_PATH)*.out $(BINARY_PATH)*.o prf.out
