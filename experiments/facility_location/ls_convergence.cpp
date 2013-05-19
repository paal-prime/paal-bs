#include <cassert>
#include <iostream>
#include <random>

#include "facility_location/FLWalker.h"
#include "facility_location/FLApxWalker.h"
#include "facility_location/SimpleFormat.h"
#include "facility_location/util.h"

#include "paal/search.h"
#include "paal/ProgressCtrl.h"
#include "paal/StepCtrl.h"
#include "paal/FIDiagram.h"

typedef facility_location::SimpleFormat<double> Instance;

struct FLAlgo
{
    typedef std::vector<bool> FacilitySet;
	
	FLAlgo() : random(786234), progress_ctrl(100) {}
	const Instance *instance;
	std::mt19937 random;

	paal::IterationCtrl progress_ctrl;
};

/** @brief [implements Algo] */
struct RandomSearch : FLAlgo
{
  template<typename Logger> double run(Logger &logger)
  {
    using namespace facility_location;
	size_t n = instance->facilities_count();
	double fitness_best = std::numeric_limits<double>::infinity();
    while (progress_ctrl.progress(fitness_best) < 1)
    {
      logger.log(fitness_best);
      FacilitySet fs(n);
	  for(size_t size = random()%n; size--;) fs[random()%n] = 1;
      fitness_best = std::min(fitness_best, fitness(*instance, fs));
    }
    return fitness_best / instance->optimal_cost();
  }
};

/** @brief [implements Algo] */
struct RandomStepLS : FLAlgo
{
  template<typename Logger> double run(Logger &logger)
  {
    using namespace facility_location;
    FacilitySet fs(instance->facilities_count());
    FLWalker<Instance> walker(*instance, fs);
    paal::HillClimb step_ctrl;
    paal::search(walker, random, progress_ctrl, step_ctrl, logger);
    return walker.current_fitness() / instance->optimal_cost();
  }
};

/** @brief [implements Algo] */
struct BestStepLS : FLAlgo
{
  template<typename Logger> double run(Logger &logger)
  {
    using namespace facility_location;
    FacilitySet fs(instance->facilities_count());
    FLApxWalker<Instance> walker(*instance, fs);
    paal::HillClimb step_ctrl;
    paal::search(walker, random, progress_ctrl, step_ctrl, logger);
    return walker.current_fitness() / instance->optimal_cost();
  }
};

int main(int argc, char **argv)
{
	assert(argc>1);
	RandomSearch rs;
	RandomStepLS rls;
	BestStepLS bls;
	Instance inst(argv[1]);
	paal::FIDiagram dia(inst.optimal_cost());
	rs.instance = rls.instance = bls.instance = &inst;
	dia.test("Random Search",rs);
	dia.test("Random Step LS",rls);
	dia.test("Best Step LS",bls);
	dia.dump(std::cout);
}
