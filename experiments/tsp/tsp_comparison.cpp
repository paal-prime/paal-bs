#include <random>
#include <algorithm>
#include <vector>
#include <iostream>  // NOLINT(readability/streams)

#include <paal/GridTable.h>

#include <result_dir.h>
#include <format.h>

#include "experiments/tsp/Algo.h"

int main(int argc, char **argv)
{
  using namespace mcts;
  Dir resdir(argc, argv);

  paal::GridTable table;
  table.push_algo("Optimal");
  table.push_algo("HillClimb");
  table.push_algo("MonteCarloSearch");
  table.push_algo("Christofides");

  tsp::TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  size_t it = 0;
  std::vector<int> samplelimit = {1000, 400, 200, 70, 32, 11, 0};
  for (auto gid : {"eil51", "eil76", "eil101", "pr152", "d198", "a280", "brd14051", })
  {
    Matrix matrix;
    dir.graphs[gid].load(matrix);

    table.columns.push_back(format("%", gid));
    table.records[0].results.push_back(dir.graphs[gid].optimal_fitness);

    double start, stop;

    ChristofidesAlgo christ(matrix);
    start = paal::realtime_sec();
    table.records[3].test(christ);
    stop = paal::realtime_sec();
    std::cerr << "runtime " << stop - start << std::endl;

    HillTimeAlgo hillclimb(matrix, std::max(0.5, stop - start));
    start = paal::realtime_sec();
    table.records[1].test(hillclimb);
    stop = paal::realtime_sec();
    std::cerr << "runtime " << stop - start << std::endl;

    MCTSAlgo<PolicyRandMean<std::mt19937> > mctsalgo(
      PolicyRandMean<std::mt19937>(random_), samplelimit[it]);
    State state(matrix, matrix.size1() / 4);
    mctsalgo.state_ = &state;
    start = paal::realtime_sec();
    if (samplelimit[it])
      table.records[2].test(mctsalgo);
    else
      table.records[2].results.push_back(1. / 0.);
    stop = paal::realtime_sec();
    std::cerr << "runtime " << stop - start << std::endl;

    std::cerr << "done " << gid << std::endl;
    it++;
  }
  std::ofstream tex(resdir("eil_graphs.tex"));
  table.dump_tex(tex);
  return 0;
}
