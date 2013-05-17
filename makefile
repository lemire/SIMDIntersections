.SUFFIXES:
#
.SUFFIXES: .cpp .o .c .h
# replace the YOURCXX variable with a path to a C++11 compatible compiler.
ifeq ($(INTEL), 1)
# if you wish to use the Intel compiler, please do "make INTEL=1".
    YOURCXX ?= /opt/intel/bin/icpc
    CXXFLAGS = -std=c++0x -O3 -Wall -xSSE4.1 -DNDEBUG=1  -ggdb
else 
    YOURCXX ?= g++-4.7
ifeq ($(DEBUG),1)
    CXXFLAGS = -msse4.2 -march=native -std=c++0x -Weffc++ -pedantic -ggdb -Wall -Wextra -Wcast-align  
else
    CXXFLAGS = -msse4.2 -march=native -std=c++0x -Weffc++ -pedantic -O3 -Wall -Wextra -Wcast-align  
endif
endif

CXX := $(YOURCXX)




HEADERS= $(shell ls include/*h)

all:  testintersection

testintersection: $(HEADERS) src/testintersection.cpp  
	$(CXX) $(CXXFLAGS) -Iinclude -o testintersection src/testintersection.cpp  



clean: 
	rm -f *.o testintersection
