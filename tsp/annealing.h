//Grzegorz Prusak
#ifndef tsp_annealing_h
#define tsp_annealing_h

#include <cmath>

namespace tsp
{
	
	template<typename Random> struct BoltzmannDistr
	//steps left / steps overall is the temperature
	//http://pl.wikipedia.org/wiki/Rozk%C5%82ad_Boltzmanna
	{
		BoltzmannDistr(double _temp, double _temp_step, Random &_random) :
			temp(_temp), temp_step(_temp_step), random(_random) {}
		double temp, temp_step;
		Random &random;
		bool operator()(double df)
		{
			//std::cout << df << " | " << exp(-df/temp) << '\n';
			return double(random())/random.max() <= exp(-df/(temp-=temp_step));
		}
	};

	template<typename Random> BoltzmannDistr<Random> boltzmann_distr(
		double temp, double temp_step, Random &random)
	{ return BoltzmannDistr<Random>(temp,temp_step,random); }

	template<typename Walker, typename Monitor>
		void random_walk(Walker &walker, Monitor &monitor)
	{
		while(monitor(walker))
		{ walker.prepare(); walker.step(); }
	}

	template<typename Walker, typename Monitor>
		void hill_climb(Walker &walker, Monitor &monitor)
	{
		while(monitor(walker))
		{
			walker.prepare();
			if(walker.new_fitness<=walker.best_fitness) walker.step();
		}
	}

	template<typename Walker, typename Monitor, typename Temperature>
		void annealing(Walker &walker, Monitor &monitor, Temperature &temp)
	{
		while(monitor(walker))
		{
			walker.prepare();
			if(temp(walker.new_fitness-walker.next_fitness)) walker.step();
		}
	}
}

#endif
