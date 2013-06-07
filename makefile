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
    CXXFLAGS = -msse4.2 -std=c++0x -Weffc++ -pedantic -ggdb -Wall -Wextra -Wcast-align  -Winline
else
    CXXFLAGS = -msse4.2 -std=c++0x -Weffc++ -DNDEBUG=1 -pedantic -O3 -Wall -Wextra -Winline -Wcast-align  
endif
endif

CXX := $(YOURCXX)




HEADERS= $(shell ls include/*h)

all:  testintersection realintersection

testintersection: $(HEADERS) src/testintersection.cpp  
	$(CXX) $(CXXFLAGS) -Iinclude -o testintersection src/testintersection.cpp  

realintersection: $(HEADERS) src/realintersection.cpp  
	$(CXX) $(CXXFLAGS) -Iinclude -o realintersection src/realintersection.cpp  


clean: 
	rm -f *.o testintersection realintersection
