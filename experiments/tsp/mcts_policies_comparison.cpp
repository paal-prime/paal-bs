#include <paal/ProgressCtrl.h>
#include <paal/GridTable.h>
#include <mcts/MonteCarloTree.h>
#include <mcts/Policy.h>
#include <tsp/TSPLIB.h>
#include <tsp/MCTS_tsp.h>

#include <result_dir.h>
#include <format.h>

#include <random>
#include <algorithm>
#include <vector>
#include <iostream>  // NOLINT(readability/streams)

typedef tsp::TSPLIB_Matrix Matrix;
typedef tsp::TSPState<Matrix> State;
typedef std::mt19937 Random;
Random random_;

/** @brief [implements Algo] */
template<typename Policy> struct Algo
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

int main(int argc, char **argv)
{
  using namespace mcts;
	Dir resdir(argc, argv);

  tsp::TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  for (auto gid : {"eil51", "eil76", "eil101"})
  {
    paal::GridTable table;
    table.push_algo("Optimal");
    table.push_algo("PolicyRandMean");
    auto randmean = make_algo(PolicyRandMean<Random>(random_));
    table.push_algo("PolicyEpsMean");
    auto epsmean = make_algo(PolicyEpsMean<Random>(random_));
    table.push_algo("PolicyEpsBest");
    auto epsbest = make_algo(PolicyEpsBest<Random>(random_));
    table.push_algo("PolicyMuSigma");
    auto musigma = make_algo(PolicyMuSigma<Random>(random_));

    Matrix matrix;
    dir.graphs[gid].load(matrix);

    for (size_t limit = matrix.size1(); limit > matrix.size1() / 11; limit /= 2) {
      State state(matrix, limit);
      randmean.state_ = epsmean.state_ = epsbest.state_ = musigma.state_ = &state;
      table.columns.push_back(format("%, limit %", gid, limit));
      table.records[0].results.push_back(dir.graphs[gid].optimal_fitness);
      double start;
      start = paal::realtime_sec();
      table.records[1].test(randmean);
      std::cerr << "runtime " << paal::realtime_sec() - start << std::endl;
      start = paal::realtime_sec();
      table.records[2].test(epsmean);
      std::cerr << "runtime " << paal::realtime_sec() - start << std::endl;
      start = paal::realtime_sec();
      table.records[3].test(epsbest);
      std::cerr << "runtime " << paal::realtime_sec() - start << std::endl;
      start = paal::realtime_sec();
      table.records[4].test(musigma);
      std::cerr << "runtime " << paal::realtime_sec() - start << std::endl;
      std::cerr << "done " << gid << " / " << limit << std::endl;
    }
    std::ofstream tex(resdir(format("%.tex", gid)));
    table.dump_tex(tex);
  }
  return 0;
}
