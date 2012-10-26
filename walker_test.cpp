//Grzegorz Prusak
#include <iostream>
#include <random>

#include "tsp/annealing.h"
#include "tsp/Cycle.h"
#include "tsp/LazyCycleWalker.h"
#include "tsp/TSPLIB.h"

#include "format.h"
#include "Stopwatch.h"

struct TimeMonitor
{
	TimeMonitor(double _time, double _optimal_fitness) :
		time(_time), steps(0), optimal_fitness(_optimal_fitness)
	{ sw.start(); }
	
	Stopwatch sw;
	uint32_t steps;
	double time;
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

int main(int argc, char **argv)
{
	if(argc<2)
	{
		std::cout << format("usage: % [TSPLib dir]\n",argv[0]);
		return 0;
	}
	
	std::mt19937 random(7873492);
	tsp::TSPLIB_Directory dir(argv[1]);
	tsp::TSPLIB_Matrix mtx;
	auto &graph = dir.graphs[20];
	graph.load(mtx);
	std::cout << graph.filename << std::endl;

	tsp::DynCycle cycle;
	tsp::cycle_shuffle(cycle,mtx.size1(),random);
	tsp::LazyCycleWalker<tsp::TSPLIB_Matrix,std::mt19937>
		walker(mtx,cycle,random);
	uint32_t steps = 2000000;
	StepsMonitor monitor(steps, graph.optimal_fitness);
	auto temp = tsp::boltzmann_distr(1.1,1/steps,random);
	tsp::annealing(walker,monitor,temp);
	//tsp::hill_climb(walker,monitor);
	return 0;
}

