.SUFFIXES:
#
.SUFFIXES: .cpp .o .c .h
# replace the YOURCXX variable with a path to a C++11 compatible compiler.
ifeq ($(INTEL), 1)
# if you wish to use the Intel compiler, please do "make INTEL=1".
    CXX ?= /opt/intel/bin/icpc
    CXXFLAGS = -std=c++0x -O3 -Wall -mAVX -DNDEBUG=1 -g3
else 
    CXX ?= g++-4.7
ifeq ($(DEBUG),1)
    CXXFLAGS = -march=native -mavx2 -std=c++11 -Weffc++ -pedantic -D_GLIBCXX_DEBUG -DDEBUG=1 -ggdb -Wall -Wextra -Wcast-align -Wconversion  -Winline
else
    CXXFLAGS = -mavx2 -std=c++11 -Weffc++ -DNDEBUG=1 -pedantic -O3 -Wall -Wextra -Winline  -Wcast-align  -Wconversion
endif
endif





HEADERS= $(shell ls include/*h)

all: unit testintersection realintersection getmatrix benchintersection
	echo "please run unit tests by running the unit executable"

intersection.o: src/intersection.cpp include/common.h  
	$(CXX) $(CXXFLAGS) -Iinclude -c src/intersection.cpp  

match.o: src/match.cpp include/match.h  
	$(CXX) $(CXXFLAGS) -Iinclude -c src/match.cpp  

thomaswu.o: src/thomaswu.cpp $(HEADERS)  
	$(CXX) $(CXXFLAGS) -Iinclude -c src/thomaswu.cpp  

testintersection: $(HEADERS) src/testintersection.cpp  match.o thomaswu.o intersection.o
	$(CXX) $(CXXFLAGS) -Iinclude -o testintersection src/testintersection.cpp  match.o thomaswu.o intersection.o

realintersection: $(HEADERS) src/realintersection.cpp  match.o thomaswu.o intersection.o
	$(CXX) $(CXXFLAGS) -Iinclude -o realintersection src/realintersection.cpp  match.o thomaswu.o intersection.o

getmatrix: $(HEADERS) src/getmatrix.cpp  match.o thomaswu.o intersection.o
	$(CXX) $(CXXFLAGS) -Iinclude -o getmatrix src/getmatrix.cpp  match.o thomaswu.o intersection.o

unit: $(HEADERS) src/unit.cpp  match.o thomaswu.o intersection.o
	$(CXX) $(CXXFLAGS) -Iinclude -o unit src/unit.cpp  match.o thomaswu.o intersection.o
benchintersection: $(HEADERS) src/benchintersection.cpp  match.o thomaswu.o intersection.o
	$(CXX) $(CXXFLAGS) -Iinclude -o benchintersection src/benchintersection.cpp  match.o thomaswu.o intersection.o


clean: 
	rm -f *.o testintersection realintersection getmatrix benchintersection

