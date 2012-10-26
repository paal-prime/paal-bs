//Grzegorz Prusak
//Stopwatch.h
#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <ctime>
#include <iostream>
#include <cassert>

#define ERRNO(call) assert(call!=-1)

class Stopwatch
{
	public:
		void start(){ ERRNO(clock_gettime(CLOCK_REALTIME,&start_time)); }
	
		double check()
		{
			timespec stop_time; ERRNO(clock_gettime(CLOCK_REALTIME,&stop_time));
			double v1 = start_time.tv_sec+double(start_time.tv_nsec)/1000000000;
			double v2 =  stop_time.tv_sec+double( stop_time.tv_nsec)/1000000000;
			return v2-v1;
		}
private:
		timespec start_time;
};

class AccumulatingStopwatch : public Stopwatch
{
	public:
		AccumulatingStopwatch(){ zero(); }
		void zero(){ counter = 0; sum = 0; }
		void stop(){ sum += check(); counter++; }
		int counter;
		double sum;
};

class VerboseStopwatch : public AccumulatingStopwatch
{
	public:
		VerboseStopwatch(const std::string &_msg) : msg_(_msg) { zero(); }
		~VerboseStopwatch(){ std::cout << msg_ << ": sum = " << sum << "; counter = " << counter << "; avg = " << sum/counter << '\n' << std::flush; }	
	private:
		std::string msg_;
};

#endif

