.SUFFIXES:
#
.SUFFIXES: .cpp .o .c .h
# replace the YOURCXX variable with a path to a C++11 compatible compiler.
ifeq ($(INTEL), 1)
# if you wish to use the Intel compiler, please do "make INTEL=1".
    YOURCXX ?= /opt/intel/bin/icpc
    CXXFLAGS = -std=c++0x -O3 -Wall -mAVX -DNDEBUG=1 -g3
else 
    YOURCXX ?= g++-4.7
ifeq ($(DEBUG),1)
    CXXFLAGS = -mavx -std=c++11 -Weffc++ -pedantic -ggdb -Wall -Wextra -Wcast-align -Wconversion  -Winline
else
    CXXFLAGS = -mavx -std=c++11 -Weffc++ -DNDEBUG=1 -pedantic -O3 -Wall -Wextra -Winline  -Wcast-align  -Wconversion
endif
endif

CXX := $(YOURCXX)




HEADERS= $(shell ls include/*h)

all:  testintersection realintersection getmatrix

match.o: src/match.cpp include/match.h  
	$(CXX) $(CXXFLAGS) -Iinclude -c src/match.cpp  

testintersection: $(HEADERS) src/testintersection.cpp  match.o
	$(CXX) $(CXXFLAGS) -Iinclude -o testintersection src/testintersection.cpp  match.o

realintersection: $(HEADERS) src/realintersection.cpp  match.o
	$(CXX) $(CXXFLAGS) -Iinclude -o realintersection src/realintersection.cpp  match.o

getmatrix: $(HEADERS) src/getmatrix.cpp  match.o
	$(CXX) $(CXXFLAGS) -Iinclude -o getmatrix src/getmatrix.cpp  match.o

clean: 
	rm -f *.o testintersection realintersection getmatrix

