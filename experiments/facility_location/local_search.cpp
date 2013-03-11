#include <cassert>
#include <iostream>
#include <random>

#include "facility_location/FLWalker.h"
#include "facility_location/SimpleFormat.h"

#include "paal/search.h"
#include "paal/ProgressCtrl.h"
#include "paal/StepCtrl.h"
#include "paal/SuperLogger.h"

typedef facility_location::SimpleFormat<double> Instance;

struct FLSearch
{
	FLSearch() : random(273648) {}

	const Instance *instance;
	std::mt19937 random;

	template<typename Logger> void run(Logger &logger)
	{
		std::vector<bool> fs(instance->facilities_count());
		for(auto v : fs) std::cout << v << ' '; std::cout << std::endl;
		for(auto v : fs) v = random()&1;
		for(auto v : fs) std::cout << v << ' '; std::cout << std::endl;
		facility_location::FLWalker<Instance> walker(*instance,fs);
		paal::IterationCtrl progress_ctrl(10000);
		paal::HillClimb step_ctrl;
		paal::search(walker,random,progress_ctrl,step_ctrl,logger);
	}
};

int main(int argc, char **argv)
{
	assert(argc>1);
	FLSearch algo;
	Instance instance(argv[1]);
	algo.instance = &instance;
	paal::ImprovementSuperLogger sl;
	sl.test("local_search",algo);
	sl.dump(std::cout);
	std::cout << instance.optimal_cost() << std::endl;
	return 0;
}
