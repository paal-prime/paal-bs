#include <gtest/gtest.h>

#include <tsp/TSPLIB.h>
#include <tsp/MCTS.h>
#include <tsp/MCTS_tsp.h>
#include <random>
#include <iostream>

template<typename Policy>
class MCTS_TSPPolicy : public ::testing::Test {
};

typedef ::testing::Types<tsp::TSPPolicyLCB<>, tsp::TSPPolicyRND<>,
        tsp::TSPPolicyRNDeBest<> > PoliciesList;
TYPED_TEST_CASE(MCTS_TSPPolicy, PoliciesList);

TYPED_TEST(MCTS_TSPPolicy, Performance_eil51) {
  using tsp::TSPLIB_Directory;
  using tsp::TSPLIB_Matrix;
  using tsp::TSPState;
  using tsp::TSPMove;

  TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  TSPLIB_Matrix matrix;
  dir.graphs[25].load(matrix);
  TSPState<TSPLIB_Matrix> state(matrix);
  std::mt19937 random;
  TypeParam policy(random);
  mcts::MCTS<TSPMove, TSPState<TSPLIB_Matrix>, TypeParam> mct(state, policy);
  size_t samples = 30000;
  double q = .95;
  while (!mct.state_.is_terminal()) {
    if (mct.state_.moves_count() < 10) {
      mct.state_.exhaustive_search_min();
      break;
    }
    TSPMove move = mct.search(samples);
    mct.apply(move);
    samples *= q;
  }
  std::cout << "res: " << mct.state_.cost_ << std::endl;
}
