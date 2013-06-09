#include <cassert>
#include <algorithm>
#include <iostream>
#include <limits>
#include <random>

#include "facility_location/RandomStepWalker.h"
#include "facility_location/BestStepWalker.h"
#include "facility_location/SimpleFormat.h"
#include "facility_location/PrimDualSchema.h"
#include "facility_location/util.h"
#include "facility_location/MCTS_fl.h"
#include "paal/search.h"
#include "paal/ProgressCtrl.h"
#include "paal/StepCtrl.h"
#include "paal/GridTable.h"
#include "mcts/MonteCarloTree.h"
#include "mcts/Policy.h"

#include <result_dir.h>
#include <format.h>

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
  BestStepSearch(double time_limit) : random(273648), time_limit_(time_limit) {}

  const Instance *instance;
  std::mt19937 random;
  double time_limit_;

  template<typename Logger> double run(Logger &logger)
  {
    using namespace facility_location;
    std::vector<bool> fs(instance->facilities_count());
    BestStepWalker<Instance> walker(*instance, fs);
    paal::TimeAutoCtrl progress_ctrl(time_limit_);
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

struct MonteCarlo
{
  MonteCarlo(double time_limit): random(1241222), time_limit_(time_limit) {}
  const Instance *instance;
  std::mt19937 random;
  double time_limit_;

  template<typename Logger> double run(Logger &logger)
  {
    using namespace facility_location;
    typedef FLState<Instance> State;
    typedef mcts::PolicyRandMean<std::mt19937> Policy;
    double atom = 2 * time_limit_ / ((instance->facilities_count() - 1)*(instance->facilities_count()));
    State state(*instance, (*instance)(0), instance->facilities_count(), instance->cities_count());
    Policy policy = Policy(random);
    mcts::MonteCarloTree<State, Policy> mct(state, policy);
    while (!mct.root_state().is_terminal())
    {
      double time_l = mct.root_state().left_decisions() * atom;
      paal::TimeAutoCtrl ctrl(time_l);
      auto move = mct.search(ctrl);
      mct.apply(move);
    }
    return mct.root_state().cost() / instance->optimal_cost();
  }
};

int main(int argc, char **argv)
{
  Dir resdir(argc, argv);

  double start, end;

  FL3Apx apx;
  FLRandom rnd;

  paal::GridTable table;
  table.push_algo("optimum");
  table.push_algo("reference time");
  table.push_algo("3 apx");
  table.push_algo("best step ls");
  table.push_algo("MCTS");
  table.push_algo("random");

  for (auto gid : {"1011EuclS", "2411EuclS", "2511EuclS", "1811EuclS"}) {
    Instance instance(format("UflLib/Euclid/%.txt", gid));
    rnd.instance = apx.instance = &instance;
    table.columns.push_back(gid);
    table.records[0].results.push_back(instance.optimal_cost());

    start = paal::realtime_sec();
    table.records[2].test(apx);
    end = paal::realtime_sec();
    std::cout << end - start << std::endl;

    table.records[1].results.push_back(end-start);

    BestStepSearch bls(std::max(end - start, 0.01));
    bls.instance = &instance;
    table.records[3].test(bls);

    MonteCarlo mcts(std::max(end - start, 0.1));
    mcts.instance = &instance;
    table.records[4].test(mcts);

    table.records[5].test(rnd);
  }
  std::ofstream tex(resdir(format("FLcomp.tex")));
  table.dump_tex(tex);
  return 0;
}
