//Grzegorz Prusak
#ifndef CycleWalker_h
#define CycleWalker_h

#include <cassert>

#include "tsp/Cycle.h"
#include "tsp/util.h"

namespace tsp
{
	template<typename Matrix, typename Random> struct CycleWalker
	{
		template<typename Cycle>
			CycleWalker(const Matrix &_matrix, Cycle &cycle, Random &_random) :
				matrix(_matrix), random(_random), next(cycle), best(cycle), steps(0)
		{
			assert(matrix.size1()==matrix.size2() && matrix.size1()==cycle.size());
			best_fitness = next_fitness = fitness(matrix,next);
		}
		
		const Matrix &matrix;
		Random &random;
		typename Matrix::value_type best_fitness, next_fitness;
		DynCycle next,best;

		typename Matrix::value_type new_fitness;
		Split split;
		uint32_t steps;
		
		void prepare()
		{
			size_t n = matrix.size1();
			split.generate(matrix.size1(),random);
			size_t lp = split.begin ? split.begin-1 : n-1;
			size_t rn = split.end < n ? split.end : 0;
			new_fitness = next_fitness
				-matrix(next[lp],next[split.begin]) -matrix(next[split.end-1],next[rn])
				+matrix(next[lp],next[split.end-1]) +matrix(next[split.begin],next[rn]);
			if(new_fitness<best_fitness) best_fitness = new_fitness;
		}

		void step()
		{
			cycle_reverse(next,split.begin,split.end);
			next_fitness = new_fitness;
			++steps;
		}
	};	
}

#endif

