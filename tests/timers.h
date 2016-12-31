#ifndef TIMERS_H
#define TIMERS_H

#ifdef _WIN32
#include <WinSock2.h>
#endif
#ifdef linux

#include <sys/time.h>
#endif
#include <vector>
#include "pradsort/pradsort.hpp"

using namespace prsort;
using namespace std;
double mclock();
class VTimer : public LVTimer{

protected:

        vector<double> * times;
        double *startTime;
        double *timing;
        int timerCount;
                    
public:
        VTimer();
        VTimer(int timerCount);
        ~VTimer();

        void timerStart(int timer); 


	void timerEnd(int timer);
	void add(int timer) ;        
        void finish(int timer);
        void reset();        
        double getTime(int timer);
       
};

#ifdef _WIN32
int
gettimeofday(struct timeval * tp, struct timezone * tzp);
#endif
#endif
