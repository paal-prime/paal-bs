#include <gtest/gtest.h>

#include <mcts/MonteCarloTree.h>
#include <mcts/Policies.h>
#include <tsp/TSPLIB.h>
#include <tsp/MCTS_tsp.h>
#include <paal/ProgressCtrl.h>

#include <random>
#include <iostream>  // NOLINT(readability/streams)

template<typename State, typename Policy>
mcts::MonteCarloTree<State, Policy>
run_mcts(State& state, Policy& policy, size_t samples, double progression = 1.0)
{
  mcts::MonteCarloTree<State, Policy> mct(state, policy);
  while (!mct.root_state().is_terminal())
  {
    if (mct.root_state().left_decisions() < 10)
    {
      mct.root_state().exhaustive_search_min();
      break;
    }
    paal::IterationCtrl ctrl(samples);
    typename State::Move move = mct.search(ctrl);
    mct.apply(move);
    samples *= progression;
  }
  return mct;
}

template<typename Policy>
class MCTS_Policy : public ::testing::Test {};

typedef ::testing::Types <
    mcts::PolicyMuSigma<>,
    mcts::PolicyRandMean<>,
    mcts::PolicyRandEpsMean<>,
    mcts::PolicyRandEpsBest<>
  > PoliciesList;
TYPED_TEST_CASE(MCTS_Policy, PoliciesList);

TYPED_TEST(MCTS_Policy, Performance_eil51)
{
  using tsp::TSPLIB_Directory;
  using tsp::TSPLIB_Matrix;
  using tsp::TSPState;

  TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  TSPLIB_Matrix matrix;
  dir.graphs[25].load(matrix);
  size_t samples = matrix.size1() * 900;

  std::mt19937 random;
  TSPState<TSPLIB_Matrix> state(matrix);
  TypeParam policy(random);
  auto mct = run_mcts<>(state, policy, samples, 0.95);
  std::cout << "samples: " << samples << " res: " << mct.root_state().cost_
    << std::endl;
}
