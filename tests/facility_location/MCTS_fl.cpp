#include <gtest/gtest.h>

#include <mcts/MonteCarloTree.h>
#include <mcts/Policy.h>
#include <facility_location/SimpleFormat.h>
#include <facility_location/MCTS_fl.h>
#include <paal/ProgressCtrl.h>

#include <random>
#include <iostream>  // NOLINT(readability/streams)
#include <string>
#include <algorithm>
#include <chrono>

template<typename State, typename Policy>
mcts::MonteCarloTree<State, Policy>
run_mcts(State& state, Policy& policy, size_t samples, double progression = 1.0)
{
  mcts::MonteCarloTree<State, Policy> mct(state, policy);
  while (!mct.root_state().is_terminal())
  {
    paal::IterationCtrl ctrl(samples);
    typename State::Move move = mct.search(ctrl);
    mct.apply(move);
    samples *= progression;
  }
  return mct;
}

template<typename Policy>
class MCTS_FLPolicy : public ::testing::Test {};

typedef std::mt19937 Random;

typedef ::testing::Types <
    mcts::PolicyRandMean<Random>,
    mcts::PolicyEpsMean<Random>,
    mcts::PolicyEpsBest<Random>,
    mcts::PolicyMuSigma<Random>
  > PoliciesList;
TYPED_TEST_CASE(MCTS_FLPolicy, PoliciesList);

TYPED_TEST(MCTS_FLPolicy, Performance_1011EuclS)
{
  using facility_location::FLState;
  using facility_location::SimpleFormat;

  unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
  std::string file = "UflLib/Euclid/1011EuclS.txt";
  SimpleFormat<double> matrix(file);

  size_t samples = matrix.cities_count();

  std::mt19937 random(seed1);
  FLState<SimpleFormat<double> > state(matrix, matrix(0), matrix.facilities_count(), matrix.cities_count());
  TypeParam policy(random);
  auto mct = run_mcts<>(state, policy, samples);
  std::cout << "samples: " << samples << " optimal cost: " << matrix.optimal_cost() << " res: " << mct.root_state().cost()
    << std::endl;
}
