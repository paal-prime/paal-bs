//Grzegorz Prusak
#ifndef TSP_UTIL_H_
#define TSP_UTIL_H_

#include <cassert>
#include <ctype.h>

namespace tsp
{
	template<typename Matrix, typename Cycle>
		double fitness(const Matrix &m, const Cycle &cycle)
	{
		assert(m.size1()==m.size2() && m.size1()==cycle.size());
		double f = m(cycle[cycle.size()-1],cycle[0]);
		for(size_t i=0; i<cycle.size()-1; ++i) f += m(cycle[i],cycle[i+1]);
		return f;
	}

	//generates a random cycle from values [0,n)
	template<typename Cycle, typename Random>
		void cycle_shuffle(Cycle &cycle, size_t n, Random &random)
	{
		cycle.resize(n);
		for(size_t i=0; i<n; ++i)
		{
			size_t j = random()%(i+1);
			cycle[i] = cycle[j];
			cycle[j] = i;
		}
	}

	//reverses [l,r) cycle segment
	template<typename Cycle>
		void cycle_reverse(Cycle &cycle, size_t l, size_t r)
	{ while(l<r) std::swap(cycle[l++],cycle[--r]); }

	//resizes matrix m to n x n and fills it with v values
	template<typename Matrix> void matrix_fill(
		Matrix &m, const typename Matrix::value_type &v, size_t n)
	{
		m.resize(n,n);
		for(size_t i=0; i<n; ++i) for(size_t j=0; j<n; ++j) m(i,j) = v;
	}

	//generates a symmetrical n x n matrix m
	template<typename Matrix, typename Gen>
		void matrix_gen_sym(Matrix &m, Gen gen, size_t n)
	{
		m.resize(n,n);
		for(size_t i=0; i<n; ++i)
			for(size_t j=i; j<n; ++j) m(j,i) = m(i,j) = gen();
	}

	//rotates cycle so that the first element is 0 and the second is its lower neighbour
	template<typename Cycle> void cycle_normalize(Cycle &cycle)
	{
		if(!cycle.size()) return;
		uint32_t i=0; while(cycle[i]) i++;
		if(i)
		{
			cycle_reverse(cycle,0,i);
			cycle_reverse(cycle,i,cycle.size());
			cycle_reverse(cycle,0,cycle.size());
		}
		if(cycle.size()>1 && cycle[1]>cycle[cycle.size()-1])
			cycle_reverse(cycle,1,cycle.size()); 
	}

	struct Split
	{
		uint32_t begin, end;

		//splits cycle [0,n) into 2 intervals of length at least 2.
		//resulting parts are [begin,end) and [end,begin)
		//begin < end
		//distribution is uniform

		//expected length = n/3:
		//int x(1-x) / int x = 1/3
		template<typename Random> void generate(uint32_t n, Random &random)
		{
			assert(n>3);
			begin = random()%n;
			end = begin+random()%(n-3)+2;
			if(end>n) std::swap(begin, end -= n);
		}
	};
}

#endif  // TSP_UTIL_H_

