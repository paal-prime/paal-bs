#include <random>
#include <algorithm>
#include <vector>
#include <iostream>  // NOLINT(readability/streams)

#include "tsp/TSPLIB.h"
#include "tsp/util.h"
#include "paal/ProgressCtrl.h"
#include "paal/SuperLogger.h"
#include "mcts/MonteCarloTree.h"
#include "tsp/MCTS_tsp.h"

typedef tsp::TSPLIB_Matrix Matrix;
typedef tsp::TSPState<Matrix> State;

std::mt19937 random_;

template<typename Policy> struct Algo  // implements Algo
{
  static const size_t full_count_ = 9;
  State state_;
  Policy policy_;
  double progression_;

  Algo(Matrix &matrix, Policy policy) : state_(matrix), policy_(policy)
  {
    progression_ = .95;
  }

  template<typename Logger> double run(Logger &logger) const
  {
    double begin = paal::realtime_sec();
    mcts::MonteCarloTree<State, Policy> mct(state_, policy_);
    size_t samples = mct.root_state().moves_count() * 950;
    while (!mct.root_state().is_terminal())
    {
      if (mct.root_state().moves_count() > full_count_)
      {
        paal::IterationCtrl ctrl(samples);
        typename State::Move move = mct.search(ctrl);
        mct.apply(move);
        samples = std::max<size_t>(1, samples * progression_);
      }
      else
      {
        mct.root_state().exhaustive_search_min();
      }
    }
    double end = paal::realtime_sec();
    std::cout << format("time=%\n", end - begin);
    return mct.root_state().cost_;
  }
};

template<typename Policy> Algo<Policy> make_algo(Matrix &matrix, Policy policy)
{ return Algo<Policy>(matrix, policy); }

int main()
{
  tsp::TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  std::vector<size_t> graph_ids = {25, 26};
  Matrix matrix;
  paal::SuperLogger sl;
  for (size_t gid : graph_ids)
  {
    dir.graphs[gid].load(matrix);
    sl.test(format("TSPPolicyRandMean %", dir.graphs[gid].filename),
            make_algo(matrix, tsp::TSPPolicyRandMean<>(random_)), 3);
  }
  sl.dump(std::cout);
  std::cout << std::flush;
  return 0;
}
