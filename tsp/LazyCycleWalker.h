//Grzegorz Prusak
#ifndef LazyCycleWalker_h
#define LazyCycleWalker_h

#include <cassert>

#include "tsp/LazyCycle.h"
#include "tsp/Cycle.h"
#include "tsp/util.h"

namespace tsp
{
	template<typename Matrix, typename Random> struct LazyCycleWalker
	{
		template<typename Cycle>
			LazyCycleWalker(const Matrix &_matrix, Cycle &cycle, Random &_random) :
				matrix(_matrix), random(_random), next(cycle), best(cycle)
		{
			assert(matrix.size1()==matrix.size2() && matrix.size1()==cycle.size());
			best_fitness = next_fitness = fitness(matrix,next);
		}
		
		const Matrix &matrix;
		Random &random;
		typename Matrix::value_type best_fitness, next_fitness;
		LazyCycle<uint32_t> next;
		DynCycle best;

		typename Matrix::value_type new_fitness;
		size_t l_,r_;
		
		void prepare()
		{
			size_t n = matrix.size1();
			assert(n>3); 
			l_ = random()%n;
			r_ = l_+random()%(n-3)+2;
			if(r_>n) std::swap(l_, r_ -= n);
			size_t lp = l_?l_-1:n-1, rn = r_<n?r_:0;
			new_fitness = next_fitness
				-matrix(next[lp],next[l_]) -matrix(next[r_-1],next[rn])
				+matrix(next[lp],next[r_-1]) +matrix(next[l_],next[rn]);
			if(new_fitness<best_fitness)
			{
				best_fitness = new_fitness;
				//O(n*sqrt(n))
				//best = next;
				//cycle_reverse(best,l_,r_);
			}
		}

		void step()
		{
			next.reverse(l_,r_);
			next_fitness = new_fitness;
		}

		template<typename Cycle> void jump(const Cycle &cycle)
		{
			next = cycle;
			next_fitness = fitness(matrix,next);
			if(next_fitness<best_fitness)
			{ best_fitness = next_fitness; best = next; }
		}
	};	
}

#endif

