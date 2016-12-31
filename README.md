Pradsort: Parallel Radix Sort
=============================

![Version](https://img.shields.io/badge/version-1.0-green.svg)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://opensource.org/licenses/MIT)

Overview
--------

This project provides parallel implementation of the radix sort algorithm designed to sort an array of integers.
Sorting is performed in a least significant digit (LSD) first scenario.
Implemented routine returns also permutation that can be used to sort data.
Parallel implementation of this algorithm has been defined using C++ language and OpenMP API.
Example programs were tested on Linux and Windows operating systems.

Author
------

The author of this project is Jaroslaw Krajewski.   
He is a student at Gdansk University of Technology, Faculty of Electronics, Telecommunications and Informatics.   


Faculty page: http://eti.pg.edu.pl/   
Department page: http://mwave.eti.pg.gda.pl/  


Project supervisors:   
Michal Mrozowski   
Piotr Sypek   
Adam Dziekonski   

Hardware and software requirements
----------------------------------
This software was extensively tested on server with two Intel Xeon E5 2600 processors
and with Linux operating system. Support for Windows operating system is limited
(Pradsort can be compiled for Windows operating system, however it was not tuned for it).
It should be noted that scalability of this software depends more on the number of memory controllers
than on the number of CPU cores.

Details
-------

A single function template forms an interface to the sorting algorithm.
Its definition is located in the pradsort/pradsort.hpp file and its declaration has following form:

```
template <class Ti, class Tk>
void pradsort( Ti* src, Tk* key, const int N, int bitCount, LVTimer* timers );
```

`Ti` defines type of the input data which will be sorted, e.g. `int`.   
`Tk` defines type of the keys which define new locations of the sorted data.   
`src` points to the input data array and after sorting is performed it also contains sorted data.   
`key` points to the integer array which after computations define location of the sorted data element   
(after computations element `src[i]` is moved to the `src[ key[i] ]`).   
`N` denotes length of the input data.   
`bitCount` defines the number of the sorted bits in each inner iteration.   
`timers` points to the additional structure which can be useful to monitor algorithm performance
(for normal usage it should be set to NULL, for testing usage see source files located in the tests directory).

Compilation
-----------

This project contains three examples of possible template usages:  
test32.cpp - example for sorting of 32-bit integers,  
test64.cpp - example for sorting of 64-bit integers,  
test128.cpp - example for sorting of 128-bit integers.  

These programs can be compiled in Linux using make command.

Sorting of the 32 and 64 bit integers is optimized. Sorting of the 128 bit integers is just prove of concept.

Testing
-------

After compilation is successful, you can run an example program, e.g.  
cd bin  
./test32 ../data/test5c.dat -1 2 1  
where test5c.dat file is an uncompressed file from the data directory.  

License
-------

Pradsort is an open-source C++ template licensed under the [MIT license](http://opensource.org/licenses/MIT).

