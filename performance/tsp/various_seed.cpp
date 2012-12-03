//Grzegorz Prusak
#include <iostream>
#include <random>

#include "tsp/annealing.h"
#include "tsp/CycleWalker.h"
#include "tsp/monitor.h"
#include "tsp/TSPLIB.h"

#include "format.h"

struct Stat
{
	typedef std::mt19937 Random;
	typedef tsp::CycleWalker<tsp::TSPLIB_Matrix,Random> Walker;
	typedef tsp::TimeMonitor Monitor;
	typedef tsp::BoltzmannDistr<Random> Temperature;
	typedef tsp::TSPLIB_Matrix Matrix;
	typedef tsp::DynCycle Cycle;

	void run(Matrix &mtx, double opt, double time_limit, uint32_t seed)
	{
		Random random(seed);
		Cycle cycle;
		for(int i=0; i<100; ++i)
		{
			tsp::cycle_shuffle(cycle,mtx.size1(),random);
			Temperature temp(tsp::fitness(mtx,cycle)/mtx.size1(),1e-50,random);
			Walker walker(mtx,cycle,random);
			Monitor monitor(time_limit);
			annealing(walker,monitor,temp);
			//hill_climb(walker,monitor);
			std::cerr << format("fitness=% (%) steps=%",
				walker.best_fitness,walker.best_fitness/opt,monitor.steps) << std::endl;
		}
	}
};

int main(int argc, char **argv)
{
	if(argc<3)
	{
		std::cout << format("usage: % [TSPLib dir] [graph_id]\n",argv[0]);
		return 0;
	}
	tsp::TSPLIB_Directory dir(argv[1]);
	tsp::TSPLIB_Matrix mtx;
	int graph_id = atoi(argv[2]);
	if(graph_id<0 || dir.graphs.size()<=size_t(graph_id))
	{
		std::cout << format("graph_id has to be from range [0;%)\n",
		dir.graphs.size());
		return 0;
	}
	auto &graph = dir.graphs[graph_id];
	std::cout << graph.filename << std::endl;
	graph.load(mtx);

	Stat stat;
	stat.run(mtx,graph.optimal_fitness,30,61723864);

	return 0;
}

