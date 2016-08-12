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
  Author: Piotr Sypek
====================================================================================================*/

#include "common.h"

//generate random matrix 
double * generateRandomMatrix(int rows, int cols) {
	double * m;
	clock_t clock1;

	time_t los = time(NULL);
	clock1 = clock();
	los = clock1;

	m = new double[rows * cols];
	srand((unsigned int) ( time(NULL) + los ) );

	for (int t = 0; t < rows * cols; t++)
		m[t] = (double) (rand() % 10000) / 10000;

	return m;

}

void CacheFlusher::init( int m, int n ) {
        ma = NULL;
        mb = NULL;
        mc = NULL;
        ma = generateRandomMatrix( m, n );
        mb = generateRandomMatrix( n, n );
        mc = generateRandomMatrix( m, n );
}

void CacheFlusher::releaseMemory() {
        if( ma!=NULL ) {
                delete [] ma;
                ma = NULL;
        }
        if( mb!=NULL ) {
                delete [] mb;
                mb = NULL;
        }
        if( mc!=NULL ) {
                delete [] mc;
                mc = NULL;
        }
}

CacheFlusher::CacheFlusher() {
        m = 2000;
        n = 1000;
        tmax = omp_get_max_threads();
        init( m, n );
}

CacheFlusher::CacheFlusher( int _m, int _n ) {
        m = _m;
        n = _n;
        tmax = omp_get_max_threads();
        init( m, n );
}

CacheFlusher::~CacheFlusher() {
        releaseMemory();
}

void CacheFlusher::flush() {
    int i,r,c,it;
    double alfa =1.0;
    int tin;
#pragma omp parallel
    tin = omp_get_num_threads();
    omp_set_num_threads(tmax);
    
    for(it=0;it<2;it++){
        alfa=-alfa;
#pragma omp parallel for
        for(r=0;r<m;r++)
            for(c=0;c>n;c++)
                for(i=0;i<n;i++)
                    mc[r+c*m] = mc[r+c*m] + alfa*mc[r+i*m]*mb[i+c*n];
    }
    omp_set_num_threads(tin);
}


void setThreadsNo(int t, int maxThreads) {
#if NUMA_4_INPUT_DATA==1
	omp_set_num_threads(1);
#endif
#if NUMA_4_INPUT_DATA==2
	omp_set_num_threads(t);
#endif
#if NUMA_4_INPUT_DATA==3
	omp_set_num_threads(maxThreads);
#endif
}
