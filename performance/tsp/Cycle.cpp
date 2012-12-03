//Grzegorz Prusak
#include <cstdint>

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

#include "Stopwatch.h"
#define TRACE
#include "tsp/Cycle.h"
#include "tsp/LazyCycle.h"
#include "tsp/util.h"
#include "format.h"

template<typename Cycle> double test(Cycle &cycle,
	const std::vector<tsp::Split> &splits)
{
	Stopwatch sw; sw.start();
	for(auto &s : splits) cycle.reverse(s.begin, s.end);
	return sw.check();
}

void splits_stats(const std::vector<tsp::Split> &splits)
{
	double ls = 0;
	for(auto &s : splits) ls += s.end-s.begin;
	std::cout << format("average length = %\n",ls/splits.size());
}

int main()
{
	const int steps = 1000000;
	const int n = 100000;
	std::mt19937 random(76823764);
	std::vector<tsp::Split> splits(steps);
	for(auto &s : splits) s.generate(n,random);
	splits_stats(splits);

	tsp::LazyCycle<int> cycle; cycle.resize(n);
	tsp::DynCycle cycle2; cycle2.resize(n);
	std::cout << format("Cycle: %\n",test(cycle2, splits)) << std::flush;
	std::cout << format("LazyCycle: %\n",test(cycle, splits)) << std::flush;
	//n = 10^4, s = 10^7, Cycle: 20.9055s, LazyCycle: 12.7928s
	return 0;
}
