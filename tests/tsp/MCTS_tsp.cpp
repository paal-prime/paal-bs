#include <gtest/gtest.h>

#include <mcts/MonteCarloTree.h>
#include <tsp/TSPLIB.h>
#include <tsp/MCTS_tsp.h>
#include <paal/ProgressCtrl.h>

#include <random>
#include <iostream>  // NOLINT(readability/streams)

template<typename Move, typename State, typename Policy>
mcts::MonteCarloTree<Move, State, Policy>
run_mcts(State& state, Policy& policy)
{
  mcts::MonteCarloTree<Move, State, Policy> mct(state, policy);
  size_t samples = 50000;
  double q = .95;
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
    samples *= q;
  }
  return mct;
}

template<typename Policy>
class MCTS_TSPPolicy : public ::testing::Test
{
};

/* TODO(stupaq): benchmarks for all policies
typedef ::testing::Types<tsp::TSPPolicyLCB<>, tsp::TSPPolicyRND<>,
        tsp::TSPPolicyRNDeBest<> > PoliciesList; */
typedef ::testing::Types < tsp::TSPPolicyRND<>, tsp::TSPPolicyRND<>,
        tsp::TSPPolicyRND<> > PoliciesList;
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

  std::mt19937 random;
  TSPState<TSPLIB_Matrix> state(matrix);
  TypeParam policy(random);
  auto mct = run_mcts<TSPMove>(state, policy);
  std::cout << "res: " << mct.root_state().cost_ << std::endl;
}
