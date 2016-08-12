

CC = g++
CFLAGS = -Wall -fPIC -m64   -fopenmp -O3 -g

SRCDIR = src
TESTDIR = tests
BUILDDIR = build
BINDIR = bin
TESTS = test32 test64 test128
PRADSORT_HPP =  pradsort/pradsort.hpp

SRCEXT = cpp

LIB = -lnuma 
INC = -I pradsort -I tests -D_GLIBCXX_PARALLEL -DNEBUG


all: build test32 test64 test128 

build:
	mkdir -p build

test32 : $(BUILDDIR)/test32.o $(BUILDDIR)/common.o $(BUILDDIR)/timers.o $(BUILDDIR)/testdata.o 
	$(CC) $(CFLAGS) $(LIB) $(INC) $(BUILDDIR)/common.o $(BUILDDIR)/timers.o $(BUILDDIR)/testdata.o $(BUILDDIR)/test32.o -o $(BINDIR)/test32

test64 : $(BUILDDIR)/test64.o $(BUILDDIR)/common.o $(BUILDDIR)/timers.o $(BUILDDIR)/testdata.o 
	$(CC) $(CFLAGS) $(LIB) $(INC) $(BUILDDIR)/common.o $(BUILDDIR)/timers.o $(BUILDDIR)/testdata.o $(BUILDDIR)/test64.o -o $(BINDIR)/test64

test128 : $(BUILDDIR)/test128.o $(BUILDDIR)/common.o $(BUILDDIR)/timers.o $(BUILDDIR)/testdata.o 
	$(CC) $(CFLAGS) $(LIB) $(INC) $(BUILDDIR)/common.o $(BUILDDIR)/timers.o $(BUILDDIR)/testdata.o $(BUILDDIR)/test128.o -o $(BINDIR)/test128


$(BUILDDIR)/test32.o: $(TESTDIR)/test32.cpp $(PRADSORT_HPP) $(BUILDDIR)/common.o $(BUILDDIR)/timers.o $(BUILDDIR)/testdata.o
	$(CC) $(CFLAGS)  $(INC) -c $(TESTDIR)/test32.cpp -o $(BUILDDIR)/test32.o

$(BUILDDIR)/test64.o: $(TESTDIR)/test64.cpp $(PRADSORT_HPP) $(BUILDDIR)/common.o $(BUILDDIR)/timers.o $(BUILDDIR)/testdata.o
	$(CC) $(CFLAGS)  $(INC) -c $(TESTDIR)/test64.cpp -o $(BUILDDIR)/test64.o

$(BUILDDIR)/test128.o: $(TESTDIR)/test128.cpp $(PRADSORT_HPP) $(BUILDDIR)/common.o $(BUILDDIR)/timers.o $(BUILDDIR)/testdata.o
	$(CC) $(CFLAGS)  $(INC) -c $(TESTDIR)/test128.cpp -o $(BUILDDIR)/test128.o
	

$(BUILDDIR)/common.o: $(TESTDIR)/common.cpp $(TESTDIR)/common.h
	$(CC) $(CFLAGS)  $(INC) -c $(TESTDIR)/common.cpp -o $(BUILDDIR)/common.o

$(BUILDDIR)/timers.o: $(TESTDIR)/timers.cpp $(PRADSORT) $(TESTDIR)/timers.h
	$(CC) $(CFLAGS)  $(INC) -c $(TESTDIR)/timers.cpp -o $(BUILDDIR)/timers.o

$(BUILDDIR)/testdata.o: $(TESTDIR)/testdata.cpp $(TESTDIR)/testdata.h
	$(CC) $(CFLAGS)  $(INC) -c $(TESTDIR)/testdata.cpp -o $(BUILDDIR)/testdata.o


clean: 
	rm -f $(BUILDDIR)/*

test: 
	./$(BINDIR)/test32; ./$(BINDIR)/test64; ./$(BINDIR)/test128;
	

	
