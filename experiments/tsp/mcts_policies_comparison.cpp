#include <random>
#include <algorithm>
#include <vector>
#include <iostream>  // NOLINT(readability/streams)

#include <paal/ProgressCtrl.h>
#include <paal/GridTable.h>
#include <mcts/MonteCarloTree.h>
#include <mcts/Policy.h>
#include <tsp/TSPLIB.h>
#include <tsp/MCTS_tsp.h>

typedef tsp::TSPLIB_Matrix Matrix;
typedef tsp::TSPState<Matrix> State;
typedef std::mt19937 Random;
Random random_;

template<typename Policy> struct Algo  // implements Algo
{
  State *state_;
  Policy policy_;
  size_t full_search_;
  double samples_ratio_;

  Algo(Policy policy, size_t full_search = 9, size_t samples_ratio = 500) :
    state_(NULL), policy_(policy), full_search_(full_search),
    samples_ratio_(samples_ratio) {}

  template<typename Logger> double run(Logger &logger) const
  {
    mcts::MonteCarloTree<State, Policy> mct(*state_, policy_);
    while (!mct.root_state().is_terminal())
    {
      if (mct.root_state().left_decisions() > full_search_)
      {
        size_t samples = mct.root_state().left_decisions() * samples_ratio_;
        paal::IterationCtrl ctrl(samples);
        auto move = mct.search(ctrl);
        mct.apply(move);
      }
      else
      {
        mct.root_state().exhaustive_search_min();
      }
    }
    return mct.root_state().cost_;
  }
};

template<typename Policy> Algo<Policy> make_algo(Policy policy)
{ return Algo<Policy>(policy); }

int main()
{
  using namespace mcts;
  paal::GridTable table;
  table.push_algo("optimum");
  table.push_algo("PolicyRandMean");
  auto randmean = make_algo(PolicyRandMean<Random>(random_));
  table.push_algo("PolicyEpsMean");
  auto epsmean = make_algo(PolicyEpsMean<Random>(random_));
  table.push_algo("PolicyEpsBest");
  auto epsbest = make_algo(PolicyEpsBest<Random>(random_));
  table.push_algo("PolicyMuSigma");
  auto musigma = make_algo(PolicyMuSigma<Random>(random_));

  tsp::TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  for (auto gid : {"eil51", "eil76", "eil101"})
  {
    Matrix matrix;
    dir.graphs[gid].load(matrix);
    State state(matrix);
    randmean.state_ = epsmean.state_ = epsbest.state_ = musigma.state_ = &state;
    table.records[0].results.push_back(dir.graphs[gid].optimal_fitness);
    table.records[1].test(randmean);
    table.records[2].test(epsmean);
    table.records[3].test(epsbest);
    table.records[4].test(musigma);
    std::cerr << "done " << gid << std::endl;
  }
  table.dump(std::cout);
  return 0;
}
