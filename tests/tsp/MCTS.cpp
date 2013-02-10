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
  std::mt19937 random;
  TSPPolicy<std::mt19937> policy(random);
  mcts::MCTS < TSPMove, TSPState<TSPLIB_Matrix>,
       TSPPolicy<std::mt19937> > mct(state, policy);
  size_t samples = 75000;
  double q = .85;
  while (!mct.state_.is_terminal()) {
    TSPMove move = mct.search(samples);
    mct.apply(move);
    samples *= q;
    std::cout << move << '\n';
  }
  std::cout << "res: " << mct.state_.cost_ << std::endl;
}
