#include <gtest/gtest.h>

#include <tsp/TSPLIB.h>
#include <tsp/MCTS.h>
#include <tsp/MCTS_tsp.h>
#include <random>
#include <iostream>

TEST(MCTS, TSPBuildPath) {
  using namespace tsp;
  TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  TSPLIB_Matrix matrix;
  dir.graphs[25].load(matrix);
  TSPState<TSPLIB_Matrix> state(matrix);
  state.apply(TSPMove(matrix.size1() - 1));
  std::mt19937 random;
  TSPPolicy<std::mt19937> policy(random);
  mcts::MCTS < TSPMove, TSPState<TSPLIB_Matrix>,
       TSPPolicy<std::mt19937> > mct(state, policy);
  size_t samples = 20000;
  double q = .95;
  while (!mct.state_.is_terminal()) {
    if (mct.state_.moves_count() < 10) {
      mct.state_.exhaustive_search_min();
      break;
    }
    TSPMove move = mct.search(samples);
    std::cerr << mct;
    mct.apply(move);
    samples *= q;
    std::cout << "move: " << move << "\n\n";
  }
  std::cout << "res: " << mct.state_.cost_ << std::endl;
}
