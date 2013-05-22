#include <cassert>
#include <algorithm>
#include <iostream>
#include <limits>
#include <random>
#include <boost/filesystem.hpp>

#include "facility_location/RandomStepWalker.h"
#include "facility_location/BestStepWalker.h"
#include "facility_location/SimpleFormat.h"
#include "facility_location/PrimDualSchema.h"
#include "facility_location/util.h"

#include "paal/search.h"
#include "paal/ProgressCtrl.h"
#include "paal/StepCtrl.h"
#include "paal/GridTable.h"

typedef facility_location::SimpleFormat<double> Instance;

struct FLRandom
{
  FLRandom() : random(782934) {}
  const Instance *instance;
  std::mt19937 random;

  typedef std::vector<bool> FacilitySet;

  template<typename Logger> double run(Logger &logger)
  {
    using namespace facility_location;
	size_t n = instance->facilities_count();
    paal::TimeAutoCtrl progress_ctrl(1.);
    double fitness_best = std::numeric_limits<double>::infinity();
    while (progress_ctrl.progress(fitness_best) < 1)
    {
      logger.log(fitness_best);
      std::vector<bool> fs(n);
	  for(size_t size = random()%n; size--;) fs[random()%n] = 1;
      fitness_best = std::min(fitness_best, fitness(*instance, fs));
    }
    return fitness_best / instance->optimal_cost();
  }

};

struct RandomStepSearch
{
  RandomStepSearch() : random(273648) {}

  const Instance *instance;
  std::mt19937 random;

  template<typename Logger> double run(Logger &logger)
  {
    using namespace facility_location;
    std::vector<bool> fs(instance->facilities_count());
    RandomStepWalker<Instance> walker(*instance, fs);
    paal::TimeAutoCtrl progress_ctrl(1.);
    paal::HillClimb step_ctrl;
    paal::search(walker, random, progress_ctrl, step_ctrl, logger);
    return walker.current_fitness() / instance->optimal_cost();
  }
};

struct BestStepSearch
{
  BestStepSearch() : random(273648) {}

  const Instance *instance;
  std::mt19937 random;

  template<typename Logger> double run(Logger &logger)
  {
    using namespace facility_location;
    std::vector<bool> fs(instance->facilities_count());
    BestStepWalker<Instance> walker(*instance, fs);
    paal::TimeAutoCtrl progress_ctrl(1.);
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
    return solver().first / instance->optimal_cost();
  }
};

int main(int argc, char **argv)
{
  const char *path = argc<2 ? "UflLib/Euclid/" : argv[1];

  RandomStepSearch ls;
  BestStepSearch ls2;
  FL3Apx apx;
  FLRandom rnd;
  paal::GridTable table;
  table.push_algo("optimum");
  table.push_algo("local search");
  table.push_algo("ls fast");
  table.push_algo("3 apx");
  table.push_algo("random");
  for (boost::filesystem::directory_iterator it(path);
       it != boost::filesystem::directory_iterator(); ++it)
    if (it->path().extension().native() == ".txt")
    {
      Instance instance(it->path().native());
      rnd.instance = ls2.instance = ls.instance = apx.instance = &instance;
      table.records[0].results.push_back(instance.optimal_cost());
      table.records[1].test(ls);
      table.records[2].test(ls2);
      table.records[3].test(apx);
      table.records[4].test(rnd);
      std::cout << "done " << it->path().native() << std::endl;
    }
  table.dump(std::cout);
  return 0;
}
