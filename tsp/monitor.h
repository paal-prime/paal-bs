//Grzegorz Prusak
#ifndef TSP_MONITOR_H_
#define TSP_MONITOR_H_

#include "format.h"
#include "Stopwatch.h"

namespace tsp {

struct TimeMonitor
{
	TimeMonitor(double _time, double _optimal_fitness) :
		time(_time), steps(0), optimal_fitness(_optimal_fitness)
	{ sw.start(); }
	
	Stopwatch sw;
	double time;
	uint32_t steps;
	double optimal_fitness;
	
	template<typename Walker> bool operator()(const Walker &walker)
	{
		if(!(steps++%10000))
			std::cout << format("fitness = %; opt = %; now = %\n",
			walker.best_fitness,optimal_fitness,walker.new_fitness);
		if(sw.check()>time)
		{
			std::cout << format("% steps\n",steps);
			return 0;
		}
		return 1;
	}
};

struct StepsMonitor
{
	StepsMonitor(uint32_t _steps, double _optimal_fitness) :
		steps(_steps), optimal_fitness(_optimal_fitness) {}
	uint32_t steps;
	double optimal_fitness;
	template<typename Walker> bool operator()(const Walker &walker)
	{
		if(!(steps%10000))
			std::cout << format("fitness = %; opt = %; now = %\n",
			walker.best_fitness,optimal_fitness,walker.new_fitness);
		return steps--;
	}
};

}

#endif  // TSP_MONITOR_H_

