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



#include "timers.h"


#include <cstdlib>

VTimer::VTimer(){};

VTimer::VTimer(int timerCount){
  timing = new double[timerCount];
  startTime = new double[timerCount];
  times = new vector<double>[timerCount];
  this->timerCount = timerCount;
  reset();

}

VTimer::~VTimer(){
            delete[] timing;
            delete[] startTime;
            delete[] times;
         }

void VTimer::timerStart(int timer) {
            startTime[timer] = mclock();
        }

        void VTimer::timerEnd(int timer) {
            double time;
            time = mclock() - startTime[timer];
            times[timer].push_back(time);
        }

        void VTimer::add(int timer) {
                timing[timer] += mclock() - startTime[timer];
        }

        void VTimer::finish(int timer){
            times[timer].push_back(timing[timer]);
            timing[timer]=0;
        }

        void VTimer::reset(){
            for(int i=0;i<timerCount;i++){
                timing[i]=0;
                startTime[i]=0;
               times[i].clear();
            }
       }
 double VTimer::getTime(int timer)
        {
            vector<double>::iterator it;
            double min,max,sum=0;
            int len = times[timer].size();

            if(len==0)return 0;

            min=max = times[timer].front();
            for(it=times[timer].begin();it<times[timer].end();it++){
                if(max<*it) max = *it;
                if(min>*it) min=*it;
                sum+=*it;
            }
            if(len<5)return sum;
            return(sum-min-max)/(len/2);
      }

double mclock() {
	struct timeval tp;

	double sec, usec;
	gettimeofday(&tp, NULL);

	sec = double(tp.tv_sec);

	usec = double(tp.tv_usec) / 1E6;
	return sec + usec;
}

#ifdef _WIN32

int gettimeofday(struct timeval * tp, struct timezone * tzp)
{

	const unsigned __int64 epoch = ((unsigned __int64)116444736000000000ULL);
	FILETIME    file_time;
	SYSTEMTIME  system_time;
	ULARGE_INTEGER ularge;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	ularge.LowPart = file_time.dwLowDateTime;
	ularge.HighPart = file_time.dwHighDateTime;

	tp->tv_sec = (long)((ularge.QuadPart - epoch) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);

	return 0;
}
#endif
