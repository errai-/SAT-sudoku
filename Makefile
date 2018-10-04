.SECONDEXPANSION:
.DELETE_ON_ERROR:

CXX=g++
CXXFLAGS=-std=c++14 -O2 -march=native 
VPATH=include:src

INCLUDE=$(PWD)/include

all: sat

sat: main.cpp Cond.o Tribools.o SudoHold.o helper.h
	cd src && $(CXX) $(CXXFLAGS) -I$(INCLUDE) -o ../$@ main.cpp SudoHold.o Tribools.o Cond.o

Cond.o: Cond.h Cond.cpp helper.h
	cd src && $(CXX) $(CXXFLAGS) -I$(INCLUDE) -c Cond.cpp

Tribools.o: Tribools.h Tribools.cpp helper.h
	cd src && $(CXX) $(CXXFLAGS) -I$(INCLUDE) -c Tribools.cpp

SudoHold.o: SudoHold.h SudoHold.cpp helper.h
	cd src && $(CXX) $(CXXFLAGS) -I$(INCLUDE) -c SudoHold.cpp

clean:
	rm -f src/*.o && rm -f ./sat
