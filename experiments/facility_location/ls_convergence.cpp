#include <cassert>
#include <iostream>
#include <random>

#include "facility_location/PrimDualSchema.h"
#include "facility_location/RandomStepWalker.h"
#include "facility_location/BestStepWalker.h"
#include "facility_location/SimpleFormat.h"
#include "facility_location/util.h"

#include "paal/search.h"
#include "paal/ProgressCtrl.h"
#include "paal/StepCtrl.h"
#include "paal/FIDiagram.h"

#include "result_dir.h"
#include "format.h"

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
    RandomStepWalker<Instance> walker(*instance, fs);
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
    BestStepWalker<Instance> walker(*instance, fs);
    paal::HillClimb step_ctrl;
    paal::search(walker, random, progress_ctrl, step_ctrl, logger);
    return walker.current_fitness() / instance->optimal_cost();
  }
};

struct FL3Apx
{
  const Instance *instance;

  template<typename Logger> double run(Logger &logger)
  {
    facility_location::PrimDualSchema<Instance> solver(*instance);
    double result = solver().first;
	logger.log(result);
	return result;
  }
};

int main(int argc, char **argv)
{
	Dir dir(argc,argv);

	std::vector<std::string> in = 
	{
		"FLClustered/test0.txt",
		"FLClustered/test1.txt",
		"FLClustered/test2.txt",
		"FLClustered/test3.txt",
		"FLClustered/test4.txt",
		"FLClustered/test5.txt",
    "UflLib/Euclid/1011EuclS.txt",
    "UflLib/Euclid/1111EuclS.txt",
    "UflLib/Euclid/111EuclS.txt",
    "UflLib/Euclid/1911EuclS.txt",
	};

  RandomStepLS rls;
	BestStepLS bls;
	//FL3Apx apx;
	for(size_t i=0; i<in.size(); ++i)
	{
		paal::FIDiagram dia(0);
		Instance inst(in[i]);
		//apx.instance = &inst;
		rls.instance = &inst;
    bls.instance = &inst;
		//dia.test("apx",apx);
		dia.test("random step ls",rls);
    dia.test("best step ls",bls);
		std::ofstream tex(dir(format("c%.tex",i)));
		dia.dump_tex(tex,in[i],format("fl_conv%",i));
	}
}
