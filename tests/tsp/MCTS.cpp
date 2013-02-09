#include <gtest/gtest.h>

#include <tsp/TSPLIB.h>
#include <tsp/MCTS.h>
#include <tsp/MCTS_tsp.h>
#include <iostream>

TEST(MCTS, TSPBuildPath) {
  srand(123432143);
  tsp::TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  tsp::TSPLIB_Matrix matrix;
  dir.graphs[25].load(matrix);
  tsp::TSPState<tsp::TSPLIB_Matrix> state(matrix);
  assert(state.moves().size() == matrix.size1());
  mcts::MCTS<tsp::TSPMove, tsp::TSPState<tsp::TSPLIB_Matrix>, tsp::TSPPolicy>
    mct(state, tsp::TSPPolicy());
  size_t samples = 35000;
  double q = .93;
  while (!mct.state_.is_terminal()) {
    tsp::TSPMove move = mct.search(samples);
    mct.apply(move);
    samples *= q;
    std::cout << move << '\n';
  }
  std::cout << "res: " << mct.state_.cost_ << std::endl;
}
