#include <paal/GridTable.h>
#include <tsp/TSPLIB.h>

#include <result_dir.h>
#include <format.h>

#include <random>
#include <algorithm>
#include <vector>
#include <iostream>  // NOLINT(readability/streams)

#include "experiments/tsp/Algo.h"

int main(int argc, char **argv)
{
  typedef std::mt19937 Random;
  using namespace mcts;
	Dir resdir(argc, argv);

  tsp::TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  for (auto gid : {"eil51", "eil76", "eil101"})
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

    Matrix matrix;
    dir.graphs[gid].load(matrix);

    for (size_t limit = matrix.size1(); limit > matrix.size1() / 11; limit /= 2) {
      State state(matrix, limit);
      randmean.state_ = epsmean.state_ = epsbest.state_ = musigma.state_ = &state;
      table.columns.push_back(format("%, limit %", gid, limit));
      table.records[0].results.push_back(dir.graphs[gid].optimal_fitness);
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
      std::cerr << "done " << gid << " / " << limit << std::endl;
    }
    std::ofstream tex(resdir(format("%.tex", gid)));
    table.dump_tex(tex);
  }
  return 0;
}
