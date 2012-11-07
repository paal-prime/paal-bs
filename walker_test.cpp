//Grzegorz Prusak
#include <iostream>
#include <random>

#include "tsp/annealing.h"
#include "tsp/Cycle.h"
#include "tsp/CycleWalker.h"
#include "tsp/LazyCycleWalker.h"
#include "tsp/monitor.h"
#include "tsp/TSPLIB.h"

#include "format.h"

int main(int argc, char **argv)
{
	if(argc<3)
	{
		std::cout << format("usage: % [TSPLib dir]\n",argv[0]);
		return 0;
	}
	
	std::mt19937 random(7873492);
	tsp::TSPLIB_Directory dir(argv[1]);
	tsp::TSPLIB_Matrix mtx;
      auto graph_it = dir.graphs.begin();
      for (int i = 1; i < 20; ++i)
         ++graph_it;
	auto &graph = graph_it->second;
	graph.load(mtx);
	std::cout << graph.filename << std::endl;

	tsp::DynCycle cycle;
	std::cout << format("size = %\n",mtx.size1());
	tsp::cycle_shuffle(cycle,mtx.size1(),random);
	tsp::CycleWalker<tsp::TSPLIB_Matrix,std::mt19937>
		walker(mtx,cycle,random);
	uint32_t steps = 20000000;
	tsp::StepsMonitor monitor(steps, graph.optimal_fitness);
	auto temp = tsp::boltzmann_distr(1.1,1/steps,random);
	tsp::annealing(walker,monitor,temp);
	//tsp::hill_climb(walker,monitor);
	return 0;
}

