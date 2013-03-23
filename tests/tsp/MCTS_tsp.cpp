#include <gtest/gtest.h>

#include <mcts/MonteCarloTree.h>
#include <tsp/TSPLIB.h>
#include <tsp/MCTS_tsp.h>
#include <paal/ProgressCtrl.h>

#include <random>
#include <iostream>  // NOLINT(readability/streams)

template<typename Move, typename State, typename Policy>
mcts::MonteCarloTree<Move, State, Policy>
run_mcts(State& state, Policy& policy, size_t samples, double progression = 1.0)
{
  mcts::MonteCarloTree<Move, State, Policy> mct(state, policy);
  while (!mct.root_state().is_terminal())
  {
    if (mct.root_state().moves_count() < 10)
    {
      mct.root_state().exhaustive_search_min();
      break;
    }
    paal::IterationCtrl ctrl(samples);
    Move move = mct.search(ctrl);
    mct.apply(move);
    samples *= progression;
  }
  return mct;
}

template<typename Policy>
class MCTS_TSPPolicy : public ::testing::Test {};

typedef ::testing::Types <
    tsp::TSPPolicyRandMean<>,
    tsp::TSPPolicyRandEpsMean<>,
    tsp::TSPPolicyRandEpsBest<>
  > PoliciesList;
TYPED_TEST_CASE(MCTS_TSPPolicy, PoliciesList);

TYPED_TEST(MCTS_TSPPolicy, Performance_eil51)
{
  using tsp::TSPLIB_Directory;
  using tsp::TSPLIB_Matrix;
  using tsp::TSPState;
  using tsp::TSPMove;

  TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  TSPLIB_Matrix matrix;
  dir.graphs[25].load(matrix);
  size_t samples = matrix.size1() * 900;

  std::mt19937 random;
  TSPState<TSPLIB_Matrix> state(matrix);
  TypeParam policy(random);
  auto mct = run_mcts<TSPMove>(state, policy, samples, 0.95);
  std::cout << "samples: " << samples << " res: " << mct.root_state().cost_
    << std::endl;
}
