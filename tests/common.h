


#ifndef COMMON_HPP
#define COMMON_HPP

#ifdef linux
#include <parallel/algorithm>
#include <numa.h>
#include <openblas/cblas.h>
#endif

#ifdef _WIN32
#include <ctime>
#include <omp.h>
#endif
#include "testdata.h"
#include "timers.h"
#define FLUSH_CACHE 1 // 1 - turns on cleaning L2, L3 memory before calculations
//     (prevent that time will be lowered, when code will work on cached data)
//     otherwise not

#define NUMA_4_INPUT_DATA  2 // 1 - one thread arranges input data
// 2 - t          threads arranges input data
// 3 - maxThreads arranges input data

using namespace prsort;

class CacheFlusher {
	double * ma,
               * mb,
               * mc;
	int m,
	    n;
	int tmax;

	void init( int m, int n );
	void releaseMemory();
public:
	CacheFlusher();
	CacheFlusher( int _m, int _n );
	~CacheFlusher();
	void flush();
};

/**
 * Checks if vector ref and x are the same
 *
 * return value: true  - vectors have the same values,
 *               false - vectors do not have the same values.
 */
template<class Ti>
bool checkResults(const Ti * ref, const Ti * x, const int N) {
	int i;
	int error = 0;
	for (i = 0; i < N; i++) {
		if (ref[i] - x[i] != 0) {
			// printf( "diff at %8d is %8d vs. %8d\n", i, ref[i], x[i] );
			error++;
		}
	}
	if (error) {
		printf(
				"[checkResults] Error occured in sorting. Number of incorrect values: %d\n",
				error);
		return false;
	} else {
		printf("[checkResults] The result of the sort is correct.\n");
		return true;
	}
}

template<class Ti,class Tk>
bool checkKey(Ti *dataIn, Ti *sorted, Tk *key, int N)
{
	for(int i=0;i<N;i++)
	{
		if(dataIn[i]!=sorted[key[i]]) return false;
	}
	return true;

}

template <class Ti>
Ti * readVector(int N, char * fileName) {
	FILE *fp;
	if ((fp = fopen(fileName, "r")) == NULL) {
		printf("Not possible to read from a file\n");
		exit(1);
	}

	Ti *h_test = new Ti[N];
	for (int i = 0; i < N; i++) {
		fscanf(fp, "%d\n", &h_test[i]);
	}
	return h_test;
}

void setThreadsNo(int t, int maxThreads);

#endif
