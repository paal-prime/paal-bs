//Grzegorz Prusak
#ifndef TSP_MONITOR_H_
#define TSP_MONITOR_H_

#include "format.h"
#include "Stopwatch.h"

namespace tsp {

/*
concept Monitor
{
	template<typename Walker> double operator()(const Walker &walker);
	//returns time of the simulation (0 - begin, 1 - end)
};
*/

struct TimeMonitor //implements Monitor
{
	TimeMonitor(double _time) :
		time(_time), last_time(0), steps(0)
	{ sw.start(); }
	
	Stopwatch sw;
	double time,last_time;
	uint32_t steps;
	
	template<typename Walker> double operator()(const Walker &walker)
	{
		//if(!(steps++%10000))
		//	std::cout << format("fitness = %; opt = %; now = %\n",
		//	walker.best_fitness,optimal_fitness,walker.new_fitness);
		if(!(steps++%1000)) last_time = sw.check();
		return last_time/time;
	}
};

struct StepsMonitor //implements Monitor
{
	StepsMonitor(uint32_t _steps, double = 0) :
		steps(_steps), current_step(0) {}
	uint32_t steps, current_step;
	double optimal_fitness;
	template<typename Walker> double operator()(const Walker &walker)
	{
		//if(!(current_step%10000))
		//	std::cout << format("fitness = %; opt = %; now = %\n",
		//	walker.best_fitness,optimal_fitness,walker.new_fitness);
		return double(current_step++)/steps;
	}
};

}

#endif  // TSP_MONITOR_H_

