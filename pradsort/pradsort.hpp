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
#ifndef PRADSORT_HPP
#define PRADSORT_HPP

#include <cstdlib>
#ifdef linux
#include <sys/time.h>
#include <parallel/algorithm>
#include <unistd.h>
#include <string.h>
#include <syscall.h>
#include <numa.h>
#endif
#ifdef _WIN32
#include <string.h>
#include <algorithm>
#endif


#define NUMA 0 // 0 - initialization using new
// 1 - array initialized in interleaved mode
// 2 - array set to interleave mode nodes before in insertValues function

#define NODE_BIND 1 // 0 - no binding
// 1 - half of threads are bound to one NUMA node, rest to second NUMA node(eg. 12 threads, 6 are bound to first node, 6 to second)
// 2 - before 9th thread, threads are bound to first NUMA node, then to second node( eg. 12 threads, 8 are bound to first node, 4 to second)

#define CPU_BIND 1 // 0 - no binding
// 1 - threads are bound to CPU cores

#define FIRST_TOUCH 2 // 0 - no first touch policy
// 1 - inserting 0 in each array element
// 2 - inserting 0 in every 1024( 4096 byte) array element

#define PAGE_SIZE_INCLUDED 0 // data is split in 4096 bytes blocks( 1024 int elements)

using namespace std;

// Timer class created to count time for each part of algorithm. 
// There is no need to implement derivative class for full functionality.
//
namespace prsort{
class LVTimer{
 
public:
    //Start time counting for timer
    virtual void timerStart(int timer) = 0;

    //Stops time counting for timer
    virtual void timerEnd(int timer) = 0;

    //Add passed time to temporary sum
    virtual void add(int timer) = 0;

    //Finish time counting (saves sum created by add fucntion)
    virtual void finish(int timer) = 0;    
              
};
// Thread data
template <class Ti>
struct RsortData {
        
        // size of data 
        int dataSize;
        
	// number of values of each bit interval
	int *bits;

	// array of pointers that are pointing to starting index of input array
	Ti **start;

	// max value
	Ti max;

	// staring index
	int start_i;

	// number of values
	int N;

	// Numa node
	int node;
	RsortData() {
            	max = 0;
		start_i = 0;
		N = 0;
		node = 0;
	}
        void init(int bitCount){
            dataSize = 2<<bitCount;
            bits = new int[dataSize];
            start = new Ti*[dataSize];
            
        }
       

#ifdef linux
}__attribute__((aligned(64)));
#endif
#ifdef _WIN32
};
#endif

 

/* Calculating starting indices and number of values for each thread
 *
 * t - number of threads
 * N - number of elements in array
 * datas - threads data
 * first - first element of array
 * page_size = page size
 */
template<class Ti>
void set_index(int t, int N, RsortData<Ti> * datas, bool first, int page_size) {
	if (t == 1) {
		(datas)->N = N * page_size;
		if (first)
			datas->start_i = 0;
		else
			(datas)->start_i = ((datas - 1))->start_i + ((datas - 1))->N;

	} else if ((t & 1) == 1) {
		set_index(1, N / t, datas, first, page_size);
		set_index(t - 1, N - N / t, datas + 1, false, page_size);
	} else {
		set_index(t / 2, N / 2, datas, first, page_size);
		set_index(t / 2, N - N / 2, datas + t / 2, false, page_size);
	}
}

/* Binding threads to Numa nodes and CPU cores
 * First touch temporary array
 *
 * t - number of threads
 * N - number of values
 * datas - threads data
 * src - input array
 * page_size - page size
 */
template<class Ti>
void binding(int t,  int N, RsortData<Ti> *datas, Ti *src, int page_size) {
#pragma omp parallel
	{
		int id = omp_get_thread_num();
		RsortData<Ti> mydata = datas[id];
#if CPU_BIND==1
#ifdef linux
		cpu_set_t set;
		CPU_ZERO(&set);
		CPU_SET(omp_get_thread_num(), &set);
		pid_t tid = (pid_t) syscall(SYS_gettid);
		sched_setaffinity(tid, sizeof(set), &set);
#endif
#ifdef _WIN32
		HANDLE process = GetCurrentProcess();
		DWORD_PTR mask = 1 << omp_get_thread_num();
		SetProcessAffinityMask(process, mask);

#endif
#endif
#ifdef linux
#if NODE_BIND ==1
		if (id < omp_get_num_threads() / 2)
		mydata.node = 0;
		else
		mydata.node = 1;
		numa_run_on_node(mydata.node);
#elif NODE_BIND==2
		numa_run_on_node(id>=8);
#endif
#endif
		Ti * start = src + mydata.start_i;
#if FIRST_TOUCH>0
#if FIRST_TOUCH==1
		for (int i = 0; i < mydata.N; i++)
#elif FIRST_TOUCH==2
#ifdef linux
		for (int i = 0; i < mydata.N; i += numa_pagesize()/sizeof(int))
#endif
#ifdef _WIN32
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);
		for (int i = 0; i < mydata.N; i += systemInfo.dwPageSize / sizeof(int))
#endif
#endif
		{
			*(start + i) = 0;
		}
#endif
	}
}

/* Counting max value
 *
 * datas - threads data
 * src - input array
 */
template<class Ti>
Ti countMax(RsortData<Ti> *datas, Ti *src) {

	Ti max = 0;
#pragma omp parallel
	{
		int id = omp_get_thread_num();
		Ti my_max = 0;
		RsortData<Ti> mydata = datas[id];
		Ti *pointer = src + mydata.start_i;
		for (int i = 0; i < mydata.N; i++)
			my_max |= (*pointer++);

#pragma omp critical
			max |= my_max;

		datas[id] = mydata;

	}
	return max;
}

/* Counting the number of bits of each bit interval
 *
 * datas - threads data
 * src - input array
 * start_higher - array of indices from which each bit interval begins
 * iterator - number of bits for which value is needed to be moved
 * bitIntervals - number of bit intervals
 */
template<class Ti>
void countBitInterval(RsortData<Ti> *datas, Ti *src, int *start_higher, int iterator, int bitIntervals) {
#pragma omp parallel
	{
#pragma omp for
		for (int i = 0; i < bitIntervals - 1; i++)
			start_higher[i] = 0;

		int id = omp_get_thread_num();
		RsortData<Ti> mydata = datas[id];

		for (int i = 0; i < bitIntervals; i++)
			mydata.bits[i] = 0;

		Ti *pointer = src + mydata.start_i;
		for (int z = 0; z < mydata.N; z++) {

			mydata.bits[(*(pointer++)) >> (iterator) & (bitIntervals - 1)]++;
		}

#pragma omp critical
		for (int i = 0; i < bitIntervals; i++)
			start_higher[i] += mydata.bits[i];

		datas[id] = mydata;
	}
#pragma omp barrier
}

/* Calculating starting indices for each bits interval
 *
 * t- number of threads
 * datas - threads data
 * start_higher - array of indices from which each bits interval begins
 * start_index - starting index
 * bitIntervals - number of bit intervals
 */
template<class Ti>
void calculateStartingIndices(int t, RsortData<Ti> *datas, int *start_higher,
		int start_index, int bitIntervals) {
        int log2t = (int)( 0.2 + log(t - 1) / log(2) ); // 0.2 - rounding error elimination
	for (int d = 0; d < log2t; d++) {
#pragma omp parallel for
		for (int i = t - 1; i >= (1 << d); i -= 1 << (d + 1)) {
			for (int j = 0; j < bitIntervals; j++)
				datas[i].bits[j] += datas[i - (1 << d)].bits[j];
		}
	}
	datas[t - 1].bits[0] = start_index;
	for (int i = 1; i < bitIntervals; i++)
		datas[t - 1].bits[i] = datas[t - 1].bits[i - 1] + start_higher[i - 1];

	for (int d = log2t; d >= 0; d--) {
#pragma omp parallel for
		for (int i = t - 1; i >= (1 << (d)); i -= 1 << (d + 1)) {
			for (int j = 0; j < bitIntervals; j++) {
				datas[i - (1 << (d))].bits[j] ^= datas[i].bits[j];
				datas[i].bits[j] ^= datas[i - (1 << (d))].bits[j];
				datas[i - (1 << (d))].bits[j] ^= datas[i].bits[j];
				datas[i].bits[j] += datas[i - (1 << (d))].bits[j];
			}
		}
	}
}

/* Inserting values into output array
 *
 * datas - threads data
 * src - input array
 * dest - output array
 * iterator - number of bits for which value is needed to be moved
 * bitIntervals - number of bit intervals
 */
template<class Ti,class Tk>
void insertValues(RsortData<Ti> *datas, Ti *src, Ti *dest,Tk *src_key,Tk *dest_key, int iterator,int N,int t, int bitIntervals) {
#ifdef linux
#if NUMA==2
	numa_interleave_memory(dest,N*sizeof(Ti),numa_all_nodes_ptr);
#endif
#endif
#pragma omp parallel
	{
		int id = omp_get_thread_num();
		RsortData<Ti> mydata = datas[id];
		for (int i = 0; i < bitIntervals; i++){
			mydata.start[i] = dest + mydata.bits[i];
		}

		Ti *pointer = src + mydata.start_i;
		Tk *key_pointer = src_key + mydata.start_i;
		for (int i = 0; i < mydata.N; i++) {
			int index = (*pointer) >> (iterator) & (bitIntervals - 1);
			*(dest_key +mydata.bits[index]++) = *(key_pointer++);

			*(mydata.start[index]++) =*(pointer++);
		}



#pragma omp barrier
		;
	}
#ifdef linux
#if NUMA==2
#if NODE_BIND==0
	numa_tonode_memory(dest,N*sizeof(Ti),0);

#elif NODE_BIND==1
	numa_tonode_memory(dest,N*sizeof(Ti),1);
	numa_tonode_memory(dest,sizeof(Ti)*datas[t/2+1].start_i,0);
#elif NODE_BIND==2
	if(t<=8){
		numa_tonode_memory(dest,N*sizeof(Ti),0);}
	else
		numa_tonode_memory(dest,N*sizeof(Ti),1);
		numa_tonode_memory(dest,sizeof(Ti)*datas[8].start_i,0);
#endif
#endif
#endif
}

/* Copy array
 *
 * datas - thread data
 * dest - output array
 * src - input array
 */
template<class Ti>
void copyArray(RsortData<Ti> *datas,Ti * dest, Ti *src) {
#pragma omp parallel
	{
		int id = omp_get_thread_num();
		RsortData<Ti> mydata = datas[id];

		Ti *source = src + mydata.start_i;

		Ti *destination = dest + mydata.start_i;
		for (int i = 0; i < mydata.N; i++){
			*(destination++) = *source++;
		}
	}
}\


/* Rewrite keys
 *
 * datas - threads data
 * src - input array
 * dest - output array
 *
 */
template<class Ti,class Tk>
void rewriteKeys(Tk *dest,Tk *src,RsortData<Ti>*datas)
{
#pragma omp parallel
	{
		int id = omp_get_thread_num();
		RsortData<Ti>mydata = datas[id];
		Tk *source = src +mydata.start_i;
		Tk *destination = dest + mydata.start_i;
		for(int i=0;i<mydata.N;i++)
		{
			*(destination++)=*(source++);
		}
	}

}

/* Determine keys
 *
 * key_src - input keys array
 * key_dest - output keys array
 * datas - threads data
 * N - number of values
 *
 */
template<class Ti, class Tk>
void determineKey(Tk *key_src, Tk *key_dest,RsortData<Ti> *datas,int N)
{
#pragma omp parallel
	{
		int id = omp_get_thread_num();
		RsortData<Ti> mydata = datas[id];

		Tk *pointer = key_src + mydata.start_i;
		for (int i = 0; i < mydata.N; i++){
			key_dest[*pointer] = mydata.start_i+i;
			pointer++;
		}
	}

}

/* Proper radix sorting
 *
 * src - input values array
 * temp - temporary values array
 * key - input keys array
 * temp_key - temporary keys array
 * datas - threads data
 * N - number of values
 * bitIntervals - number of bit intervals
 */
template<class Ti,class Tk>
void RSB(Ti* src, Ti * temp,Tk *key,Tk *temp_key, RsortData<Ti> * datas, const int N, int bitCount, LVTimer *timers )
{
        int const bitIntervals = 1<<bitCount;
	Ti * t1 = src;
	Ti * t2 = temp;

	Tk * k1 = key;
	Tk * k2 = temp_key;

	//number of threads
	int t = 0;
#pragma omp parallel
	if (omp_get_thread_num()==0 )
		t = omp_get_num_threads();

	//array of indices from which each bit interval begins
	int *start_higher = new int[bitIntervals ];
	Ti max = t1[0];

	//okresla sprawdzany bit
	int iterator = 0;

	// counting max value

        if(timers!=NULL)
            timers->timerStart(1);


	max = countMax(datas, t1);

        if(timers!=NULL)
            timers->timerEnd(1);


	//main loop
	while (max>0) {

 
 
            if(timers!=NULL)
                timers->timerStart(2);


		// Counting the number of bits of each bit interval
		countBitInterval(datas, t1, start_higher, iterator,bitIntervals);

            if(timers!=NULL)
	    {
		timers->add(2);
	    	timers->timerStart(3);
	    }


 
#pragma omp barrier

		// Calculating starting indices for each bits interval
		calculateStartingIndices<Ti>( t, datas, start_higher, 0,bitIntervals);

            if(timers!=NULL)
            {
	        timers->add(3);
		timers->timerStart(4);
	    }


		// Inserting values into output array
	    insertValues(datas, t1, t2,k1,k2, iterator,N,t,bitIntervals);
            if(timers!=NULL)
    	        timers->add(4);
    

		// Changing checked bits
		max = max>>bitCount;
		Ti * iswap;
		iswap = t1;
		t1 = t2;
		t2 = iswap;
		Tk* kswap = k1;
		k1=k2;
		k2= kswap;

		iterator += bitCount;

	}

	determineKey(k1,k2,datas,N);

// Copy array values into src array in case startSrc!=src
        if(timers!=NULL)
	    timers->timerStart(5);

	if (src != t1)
		copyArray(datas, src, t1);
	if (key!= k2)
		rewriteKeys(key,k2,datas);


        if(timers!=NULL)
 	    timers->timerEnd(5);

       
        if(timers!=NULL)
	    for (int i = 2; i < 5; i++)
		timers->finish(i);

}

/*	Parallel radix sort
 *
 * src - input array of values
 * key - output array of keys
 * N - number of values
 * bitCount - number of sorted bits in one iteration
 */
template <class Ti, class Tk>

void pradsort(Ti * src,Tk *key, const int N, int bitCount ,LVTimer *timers)
{
	Ti * temp = NULL;
	Tk * temp_key = NULL;
	RsortData<Ti> * datas;
	int t = 0;
#pragma omp parallel
	if (omp_get_thread_num() == 0)
		t = omp_get_num_threads();

#ifdef linux
	posix_memalign((void**) &datas, 64, t * sizeof(RsortData<Ti>));
#endif
#ifdef _WIN32
	datas = (RsortData<Ti>*) _aligned_malloc(t*sizeof(RsortData<Ti>), 64);
#endif
	int page_size = 1;
#if PAGE_SIZE_INCLUDED== 1
#ifdef linux
	page_size= numa_pagesize()/sizeof(Ti);
#endif
#ifdef _WIN32

	SYSTEM_INFO sInfo;
	GetSystemInfo(&sInfo);
	page_size = sInfo.dwPageSize / sizeof(Ti);
#endif
#endif
	for(int i=0;i<t;i++){
            datas[i].N=0;
            datas[i].init(bitCount);
            
        }
	set_index<Ti>(t, N / page_size, datas, true, page_size);
	datas[t - 1].N += N - ((N / page_size) * page_size);

#ifdef linux
	temp_key = new Tk[N];
#if NUMA==2
#if NODE_BIND ==1
	numa_tonode_memory(src,sizeof(Ti)*datas[t/2+1].start_i,0);
#elif NODE_BIND==2 || CPU_BIND==1
	if(t<=8)
		numa_tonode_memory(src,sizeof(Ti)*N,0);
	else
		numa_tonode_memory(src,sizeof(Ti)*datas[8].start_i,0);
#endif
#endif
#if NUMA ==1

	temp = (Ti*) numa_alloc_interleaved(N * sizeof(Ti));
#elif NUMA==2

	temp = (Ti*)numa_alloc_onnode(N*sizeof(Ti),1);
	numa_tonode_memory(temp,sizeof(Ti)*(datas[t/2+1].start_i)/2,0);

#else
	temp = new Ti[N];
#endif
#endif
#ifdef _WIN32
	temp = new Ti[N];
	temp_key = new Tk[N];
#endif
        if(timers!=NULL)
	    timers->timerStart(0);


	// Binding threads to Numa nodes and CPU cores
	// First touch temporary array
	binding<Ti>(t, N, datas, temp, page_size);


        if(timers!=NULL)
	    timers->timerEnd(0);

	// proper sorting
	RSB<Ti,Tk>(src, temp,key, temp_key,datas,N,bitCount, timers);


	// Unbinding
#ifdef linux
#pragma omp parallel

	numa_run_on_node_mask(numa_all_nodes_ptr);
#endif

#pragma omp barrier
#ifdef _WIN32
	_aligned_free(datas);
#endif
#ifdef linux
	free(datas);
#endif

#ifdef linux
	delete[] temp_key;
#if NUMA>0
	numa_free(temp, N * sizeof(Ti));
#else
	delete[] temp;
#endif
#endif
#ifdef _WIN32
	delete[] temp;
#endif

}
}
#endif


