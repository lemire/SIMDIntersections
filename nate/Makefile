CFLAGS = -std=gnu99 -O2 -Wall -g3 -msse4.2
IACA = /opt/intel/iaca-lin32/bin/iaca.sh -mark 0 -64 -arch SNB

all:  libscalvec.a

test: test.o scalar.o scalvec.c
	$(CC) $(CFLAGS) test.o scalar.o -DTEST scalvec.c -o scalvec_test

debug: test.o scalar.o scalvec.c
	$(CC) $(CFLAGS) test.o scalar.o -DDEBUG -DTEST scalvec.c -o scalvec_debug


scalvec.o:  scalvec.def Makefile
	$(CC) $(CFLAGS) -c scalvec.c -o scalvec.o

iaca: scalvec.c
	$(CC) $(CFLAGS) -DIACA -c scalvec.c -o iaca.o
	$(IACA) -analysis LATENCY -o latency.iaca iaca.o
	$(IACA) -analysis THROUGHPUT -o throughput.iaca iaca.o	

libscalvec.a: scalar.o scalvec.o scalvec.def Makefile
	$(AR) $(ARFLAGS) $@ $^

clean: 
	rm -f *.o scalvec_test scalvec_debug libscalvec.a

