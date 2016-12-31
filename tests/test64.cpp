/*====================================================================================================
Copyright (c) 2016 Gdansk University of Technology

Unless otherwise indicated, Source Code is licensed under MIT license.
See further explanation attached in License Statement (distributed in the file LICENSE).

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
====================================================================================================*/
/*====================================================================================================
  Author: Jaroslaw Krajewski
====================================================================================================*/

#include "common.h"
#include "pradsort.hpp"

int main(int argc, char* argv[]) {
	printf("\n NODE_BIND:%d, NUMA:%d, CPU_BIND:%d, FIRST_TOUCH:%d\n",NODE_BIND, NUMA, CPU_BIND, FIRST_TOUCH);

        int repetitions, // number of repetition 
			maxThreads, // max number of threads
			it,
                        N; // array size;
        int bitCount = 1;
	int * key; // array of keys
	long * dataIn; // input data
	long * dataSTL; // input stl data
	long * dataRadix; // input radix data

        repetitions = 1;
#pragma omp parallel
	maxThreads = omp_get_num_threads();

        if(argc ==1 ){
            printf("prog input_file number_of_elements bit_count number_of_repetitions\n");
            printf("NO INPUT FILE");
            return 0;
        }
        if(argc == 2){
            printf("prog input_file number_of_elements bit_count number_of_repetitions\n");
            printf("NO ELEMENT COUNT\n");
            return 0;
        }
        if(argc >2 ){
	    N = (int) strtol(argv[2], NULL, 10);
        }
        if(argc >3){
             int tmp;
	    tmp = (int) strtol(argv[3], NULL, 10);
	    if ((tmp > 0) && (tmp<=16 )) // limit bit count
		bitCount = tmp;
        }        
        if(argc >4){
             int tmp;
	    tmp = (int) strtol(argv[4], NULL, 10);
	    if ((tmp > 0) && (tmp<=10000 )) // limit repetitions
		repetitions = tmp;
        }

        int *input;
	size_t N2;
	printf( "Reading data from file.\n" );
        if( readIntArrayFile( argv[1], &input, &N2 ) )
           return 1; 
	printf( "Data reading done.\n" );

        if( (N2<(size_t)N) || (N<=0) )
		N = N2;


       	printf( "\nPARALLEL STL SORT for N=%d, max threads = %d, test repetitions: %d\n", N, maxThreads, repetitions);

	dataIn = new long[N]; 
	dataSTL = new long[N];

#ifdef _WIN32

	dataRadix = new long[N];
	key = new int[N];
#endif
#ifdef linux

	key = new int[N];
#if NUMA==0

	dataRadix = new long[N]; 

#elif NUMA==1
			dataRadix = (long*) numa_alloc_interleaved(N * sizeof(long));
#elif NUMA==2
			dataRadix = (long*)numa_alloc_onnode(sizeof(long)*N,1);
#endif
#endif
	VTimer stlTimes(maxThreads);
	VTimer radixTimes(maxThreads);
#if TIME_COUNT==1
	VTimer partTimes(TIMERS_COUNT);
#endif
#if FLUSH_CACHE==1
#ifdef linux
        CacheFlusher cf;
#endif
#endif

        for(long i=0;i<N;i++)
		dataIn[i]=input[i];
	delete[] input;

// loop from 1 to maxThreads
	for (int t = 1; t <= maxThreads; t++) {
		int i;
#if TIME_COUNT==1
                partTimes.reset();
#endif
#if CALC_REF==1
// parallel STL
		for (it = 0; it < repetitions; it++) {
			setThreadsNo(t, maxThreads);
#pragma omp parallel for private(i)
			for (i = 0; i < N; i++)
				dataSTL[i] = dataIn[i];
#if FLUSH_CACHE==1
#ifdef linux
                        cf.flush();
#endif
#endif
			stlTimes.timerStart(t-1);

#ifdef linux
			__gnu_parallel::sort(dataSTL, dataSTL + N);
#endif
#ifdef _WIN32
			std::sort(dataSTL, dataSTL + N);
#endif
			stlTimes.timerEnd(t-1);
		}

#if FLUSH_CACHE==1
#ifdef linux
                cf.flush();
#endif
#endif
#endif

// radix sort V1
		for (it = 0; it < repetitions; it++) {
			setThreadsNo(t, maxThreads);
#pragma omp parallel for private(i) default(shared)

			for (i = 0; i < N; i++){
				dataRadix[i] = dataIn[i];
				key[i]=i;
			}

#if FLUSH_CACHE==1
#ifdef linux
                        cf.flush();
#endif
#endif
			omp_set_num_threads(t);
			radixTimes.timerStart(t-1);
#if TIME_COUNT==1
                        prsort::pradsort<long,int>(dataRadix,key, N, bitCount,&partTimes);
#else
                        prsort::pradsort<long,int>(dataRadix,key, N,bitCount,NULL);
#endif
			radixTimes.timerEnd(t-1);

		}

       
#if CALC_REF==1
		printf("|STL   SORT(th=%2d)  : %1.3fs  |\t", t,
				stlTimes.getTime(t-1));
#endif
#if TIME_COUNT==1
		for (int i = 0; i < TIMERS_COUNT; i++) {
#if CREATE_OUTPUT==1
			printf("%d %d %d %d %d %d %d %f\n", NUMA, NODE_BIND, CPU_BIND, FIRST_TOUCH,bitCount , t, i ,partTimes.getTime(i));
#else
			printf("part%d :%f ", i, partTimes.getTime(i));
#endif

		}
#endif
#if CREATE_OUTPUT ==1
		        printf("%d %d %d %d %d %d calosc %1.3f", NUMA,NODE_BIND,CPU_BIND,FIRST_TOUCH,bitCount, t ,radixTimes.getTime(t-1));
#else
		printf("|RADIX SORT (th=%2d)  : %1.3fs  |\t", t,
				radixTimes.getTime(t-1));
#endif

// Attention: checking result only from the last function usage 

#if CALC_REF==1
		checkResults(dataSTL, dataRadix, N);
#else
		printf("\n");
#endif

#if CHECK_KEY==1
	if(checkKey(dataIn,dataRadix,key,N))printf("Keys are good\n");

#endif
	}

#ifdef linux
	delete[] key;
#if NUMA>0
	numa_free(dataRadix, sizeof(long) * N);
        
#else

	delete[] dataRadix;
#endif
#endif
#ifdef _WIN32
	delete[] dataRadix;
#endif

	delete[] dataIn;
	delete[] dataSTL;
	
#if TIME_COUNT==1
	
        
        
#endif
	return 0;
}
