//Grzegorz Prusak
#include <iostream>
#include <random>

#include "tsp/annealing.h"
#include "tsp/CycleWalker.h"
#include "tsp/monitor.h"
#include "tsp/TSPLIB.h"

#include "format.h"

template<typename Temp> struct TempWrapper
{
	template<typename ...Args> TempWrapper(Args... args) : 
		gens(0), steps(0), temp(args...) {}
	uint32_t gens, steps;
	Temp temp;
	bool operator()(double df, double time)
	{
		bool res = temp(df,time);
		if(res) steps++;
		if(!(++gens%10000)) std::cout
			<< format("gens = %; steps = % (%)\n",gens,steps,double(steps)/gens)
			<< std::flush;
		return res;
	}
};

int main(int argc, char **argv)
{
	if(argc<2)
	{
		std::cout << format("usage: % [TSPLib dir]\n",argv[0]);
		return 0;
	}
	
	tsp::TSPLIB_Directory dir(argv[1]);
	tsp::TSPLIB_Matrix mtx;
	auto &graph = dir.graphs[20];
	graph.load(mtx);
	std::cout << graph.filename << std::endl;

	typedef std::mt19937 Random;
	Random random(64236738);
	uint32_t steps = 10000000;
	
	tsp::DynCycle cycle;
	tsp::cycle_shuffle(cycle,mtx.size1(),random);
	tsp::CycleWalker<tsp::TSPLIB_Matrix,Random> walker(mtx,cycle,random);
	
	tsp::StepsMonitor monitor(steps, graph.optimal_fitness);
	TempWrapper<tsp::BoltzmannDistr<Random>>
		//temp(tsp::fitness(mtx,cycle)/mtx.size1(),.9999,random);
		temp(tsp::fitness(mtx,cycle)/mtx.size1(),1e-300,random);
	
	tsp::annealing(walker,monitor,temp);
	return 0;
}

