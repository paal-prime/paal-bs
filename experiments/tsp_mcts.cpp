#include <paal/ProgressCtrl.h>
#include <paal/SuperLogger.h>
#include <mcts/MonteCarloTree.h>
#include <mcts/Policies.h>
#include <tsp/TSPLIB.h>
#include <tsp/MCTS_tsp.h>

#include <random>
#include <algorithm>
#include <vector>
#include <iostream>  // NOLINT(readability/streams)

typedef tsp::TSPLIB_Matrix Matrix;
typedef tsp::TSPState<Matrix> State;

std::mt19937 random_;

template<typename Policy> struct Algo  // implements Algo
{
  static const size_t full_count_ = 9;
  State state_;
  Policy policy_;
  double progression_;

  Algo(State state, Policy policy) : state_(state), policy_(policy)
  {
    progression_ = .95;
  }

  template<typename Logger> double run(Logger &logger) const
  {
    double begin = paal::realtime_sec();
    mcts::MonteCarloTree<State, Policy> mct(state_, policy_);
    size_t samples = mct.root_state().left_decisions() * 950;
    while (!mct.root_state().is_terminal())
    {
      if (mct.root_state().left_decisions() > full_count_)
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

template<typename Policy> Algo<Policy> make_algo(State state, Policy policy)
{ return Algo<Policy>(state, policy); }

int main()
{
  const size_t repeats = 1;
  tsp::TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  std::vector<size_t> graph_ids = {25, 26};
  Matrix matrix;
  paal::SuperLogger sl;
  for (size_t gid : graph_ids)
  {
    dir.graphs[gid].load(matrix);
    sl.test(format("PolicyRandMean, moves_limit=inf %",
          dir.graphs[gid].filename), make_algo(State(matrix),
            mcts::PolicyRandMean<>(random_)), repeats);
    sl.test(format("PolicyRandMean, moves_limit=30 %",
          dir.graphs[gid].filename), make_algo(State(matrix, 30),
            mcts::PolicyRandMean<>(random_)), repeats);
    sl.test(format("PolicyRandMean, moves_limit=10 %",
          dir.graphs[gid].filename), make_algo(State(matrix, 10),
            mcts::PolicyRandMean<>(random_)), repeats);
    sl.test(format("PolicyRandMean, moves_limit=5 %",
          dir.graphs[gid].filename), make_algo(State(matrix, 5),
            mcts::PolicyRandMean<>(random_)), repeats);
  }
  sl.dump(std::cout);
  std::cout << std::flush;
  return 0;
}
