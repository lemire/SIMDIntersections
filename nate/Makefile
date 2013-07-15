CFLAGS = -std=gnu99 -O3 -Wall -g3 -msse4.2

all:  scalvec_def.o

test: test.o scalar.o scalvec_def.c
	$(CC) $(CFLAGS) test.o scalar.o -DTEST scalvec_def.c -o scalvec_test

scalvec.o: scalar.o

clean: 
	rm -f *.o scalvec_test

