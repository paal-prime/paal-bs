//Grzegorz Prusak
#ifndef TSP_ANNEALING_H_
#define TSP_ANNEALING_H_

#include <cmath>

namespace tsp
{

/*
concept Temperature
{
	bool operator()(double fitness_delta, double time);
	//fitness_delta = new_fitness - current_fitness
	//time \in [0;1] (0 - begin, 1 - end of annealing)
	//returns decision wheter to make a step in the simulation
};
*/

template<typename Random> struct BoltzmannDistr //implements Temperature
//http://pl.wikipedia.org/wiki/Rozk%C5%82ad_Boltzmanna
{
	BoltzmannDistr(double _t0, double _t1, Random &_random) :
		t0(_t0), t1(_t1), random(_random), steps(0) {}
	double t0,t1,f;
	Random &random;
	uint32_t steps;
	bool operator()(double df, double time)
	{
		if(!(steps++%100)) f = pow(t0/t1,time)/t0;
		return double(random())/random.max() <= exp(-df*f);
	}
};

template<typename Random> BoltzmannDistr<Random> boltzmann_distr(
	double t0, double t1, Random &random)
{ return BoltzmannDistr<Random>(t0,t1,random); }

template<typename Walker, typename Monitor>
	void random_walk(Walker &walker, Monitor &monitor)
{
	while(monitor(walker)<1)
	{ walker.prepare(); walker.step(); }
}

template<typename Walker, typename Monitor>
	void hill_climb(Walker &walker, Monitor &monitor)
{
	while(monitor(walker)<1)
	{
		walker.prepare();
		if(walker.new_fitness<=walker.best_fitness) walker.step();
	}
}

template<typename Walker, typename Monitor, typename Temperature>
	void annealing(Walker &walker, Monitor &monitor, Temperature &temp)
{
	while(1)
	{
		double time = monitor(walker);
		if(time>=1) break;
		walker.prepare();
		if(temp(walker.new_fitness-walker.next_fitness,time)) walker.step();
	}
}

}

#endif  // TSP_ANNEALING_H_
