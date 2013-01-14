#ifndef TSP_MONTECARLOALGO_H_
#define TSP_MONTECARLOALGO_H_

#include <cstddef>
#include "paal/ProgressCtrl.h"
#include "./format.h"

template<typename Matrix, typename Random, bool Version> struct MonteCarlo  // implements Algo
{

	MonteCarlo(Matrix &_matrix, Random &_random, double _time_sec) :
		matrix(_matrix), random(_random), time_sec(_time_sec) {}

	Matrix &matrix;
	Random &random;
	double time_sec;

	typedef std::vector<size_t> Cycle;

	template<typename T> void shuffle(T *begin, T *end)
	{
		for(ptrdiff_t i=1; i<end-begin; ++i)
			std::swap(begin[i],begin[random()%(i+1)]);
	}

	double get_fitness(Cycle &cycle, size_t i)
	{
		return matrix(cycle[i-1],cycle[i])+
			matrix(cycle[i],cycle[i<matrix.size1()-1?i+1:0]);
	}

	template<typename Logger> double run(Logger &logger)
	{
		paal::TimeCtrl progress_ctrl(time_sec,100);

		Cycle cycle;
		size_t n = matrix.size1();
		tsp::cycle_shuffle(cycle,n,random);
		double fitness = tsp::fitness(matrix,cycle);
		//for(size_t &v : cycle) std::cout << v << ' '; std::cout << std::endl;
		logger.log(fitness);
		for(size_t i=1; i<n-1; ++i)
		{
			size_t count[n];
			double sum[n];
			for(size_t j=0; j<n; ++j) sum[j] = count[j] = 0;
			while(progress_ctrl.progress(fitness)<double(i)/n)
			{
				if(Version)
				{
					//O(n) shuffler
					shuffle(&cycle[0]+i,&cycle[n]);
					fitness = tsp::fitness(matrix,cycle);
				}
				else
				{
					//O(1) leader swapper
					size_t j = random()%(n-i-1)+i+1;
					fitness -= get_fitness(cycle,i)+get_fitness(cycle,j);
					std::swap(cycle[i],cycle[j]);
					fitness += get_fitness(cycle,i)+get_fitness(cycle,j);
					//std::cout << fitness-tsp::fitness(matrix,cycle) << std::endl; 
				}
				/*if(!count[cycle[i]] || sum[cycle[i]]>fitness)
				{
					count[cycle[i]] = 1;
					sum[cycle[i]] = fitness;
				}*/
				count[cycle[i]]++;
				sum[cycle[i]] += fitness;
				logger.log(fitness);
			}
			size_t bi = -1; double bv = 1./0.;
			for(size_t j=0; j<n; ++j) if(count[j])
			{
				double nv = sum[j]/count[j];
				//std::cout << format("%(%) ",nv,count[j]);
				if(bv>nv){ bi = j; bv = nv; }
			}
			//std::cout << std::endl;
			//std::cout << format("cycle[%] = %\n",i,bi) << std::flush;
			for(size_t j=i; j<n; ++j)
				if(cycle[j]==bi) std::swap(cycle[j],cycle[i]);
			fitness = tsp::fitness(matrix,cycle);
		}
		std::cout << "it = " << logger.iterations << std::endl;
		return fitness;
	}
};

#endif  // TSP_MONTECARLOALGO_H_
