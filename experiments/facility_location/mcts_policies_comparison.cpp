#include <paal/ProgressCtrl.h>
#include <paal/GridTable.h>
#include <mcts/MonteCarloTree.h>
#include <mcts/Policy.h>
#include <facility_location/SimpleFormat.h>
#include <facility_location/MCTS_fl.h>

#include <result_dir.h>
#include <format.h>

#include <random>
#include <algorithm>
#include <vector>
#include <iostream>  // NOLINT(readability/streams)
#include <string>

typedef facility_location::SimpleFormat<double> Matrix;
typedef facility_location::FLState<Matrix> State;

typedef std::mt19937 Random;
Random random_;
size_t samples_ratio = 50;
/** @brief [implements Algo] */
template<typename Policy> struct Algo
{
  State *state_;
  Policy policy_;

  Algo(Policy policy) :
    state_(NULL), policy_(policy) {}

  template<typename Logger> double run(Logger &logger) const
  {
    mcts::MonteCarloTree<State, Policy> mct(*state_, policy_);
    while (!mct.root_state().is_terminal())
    {
      size_t samples = mct.root_state().left_decisions() * samples_ratio;
      paal::IterationCtrl ctrl(samples);
      auto move = mct.search(ctrl);
      mct.apply(move);
    }
    return mct.root_state().cost();
  }
};

template<typename Policy> Algo<Policy> make_algo(Policy policy)
{ return Algo<Policy>(policy); }

int main(int argc, char **argv)
{
  using namespace mcts;
	Dir resdir(argc, argv);
	std::string src_dir = "./UflLib/Euclid/";
  for (auto file : {"1011EuclS", "1111EuclS", "2211EuclS"})
  {
    paal::GridTable table;
    table.push_algo("Optimal");
    table.push_algo("PolicyRandMean");
    auto randmean = make_algo(PolicyRandMean<Random>(random_));
    table.push_algo("PolicyEpsMean");
    auto epsmean = make_algo(PolicyEpsMean<Random>(random_));
    table.push_algo("PolicyEpsBest");
    auto epsbest = make_algo(PolicyEpsBest<Random>(random_));
    table.push_algo("PolicyMuSigma");
    auto musigma = make_algo(PolicyMuSigma<Random>(random_));

    Matrix matrix(src_dir + file + ".txt");

    for(auto sampling : {10, 20, 50, 100}) {
      samples_ratio = sampling;
      State state(matrix, matrix(0), matrix.facilities_count(), matrix.cities_count());
      randmean.state_ = epsmean.state_ = epsbest.state_ = musigma.state_ = &state;
      table.columns.push_back(format("% / %", file, sampling));
      table.records[0].results.push_back(matrix.optimal_cost());
      double start;
      start = paal::realtime_sec();
      table.records[1].test(randmean);
      std::cerr << "runtime " << paal::realtime_sec() - start << std::endl;
      start = paal::realtime_sec();
      table.records[2].test(epsmean);
      std::cerr << "runtime " << paal::realtime_sec() - start << std::endl;
      start = paal::realtime_sec();
      table.records[3].test(epsbest);
      std::cerr << "runtime " << paal::realtime_sec() - start << std::endl;
      start = paal::realtime_sec();
      table.records[4].test(musigma);
      std::cerr << "runtime " << paal::realtime_sec() - start << std::endl;
      std::cerr << "done " << file << std::endl;
    }
    std::ofstream tex(resdir(format("%.tex", file)));
    table.dump_tex(tex);

  }
  return 0;
}
