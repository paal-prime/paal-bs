#include "tsp/TSPLIB.h"
#include "tsp/TwoOptWalker.h"
#include "tsp/util.h"
#include "paal/search.h"
#include "paal/ProgressCtrl.h"
#include "paal/StepCtrl.h"
#include "paal/SuperLogger.h"
#include <random>
#include <vector>
#include <iostream>
#include "format.h"

typedef tsp::TSPLIB_Matrix Matrix;

std::mt19937 random_(786284);


struct Algo  // implements Algo
{
  Algo(Matrix &_matrix) : matrix(_matrix) {}
  Matrix &matrix;

  template<typename Logger, typename StepCtrl, typename Cycle>
  double run_(Logger &logger, StepCtrl &step_ctrl, const Cycle &cycle) const
  {
    tsp::TwoOptWalker<Matrix> walker(matrix, cycle);
    paal::IterationCtrl progress_ctrl(10000000);
    //paal::TimeAutoCtrl progress_ctrl(15);
    //double begin = paal::realtime_sec();
    paal::search(walker, random_, progress_ctrl, step_ctrl, logger);
    //double end = paal::realtime_sec();
    //std::cout << format("time=%\n",end-begin);
    return walker.current_fitness();
  }
};

struct HillAlgo : Algo
{
  HillAlgo(Matrix &_matrix) : Algo(_matrix) {}

  template<typename Logger> double run(Logger &logger) const
  {
    std::vector<size_t> cycle;
    tsp::cycle_shuffle(cycle, matrix.size1(), random_);
    paal::HillClimb step_ctrl;
    return run_(logger, step_ctrl, cycle);
  }
};

struct AnneAlgo : Algo
{
  AnneAlgo(Matrix &_matrix) : Algo(_matrix) {}

  template<typename Logger> double run(Logger &logger) const
  {
    std::vector<size_t> cycle;
    tsp::cycle_shuffle(cycle, matrix.size1(), random_);
    paal::Annealing step_ctrl(
      tsp::fitness(matrix, cycle) / matrix.size1(), 1e-9);
    return run_(logger, step_ctrl, cycle);
  }
};

int main()
{
  tsp::TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  std::vector<std::string> graph_ids = {"brazil58", "d2103", "d15112", "fl3795",
      "pcb3038", "pla33810", "rl5934", "ulysses22"};
  Matrix matrix;
  paal::SuperLogger sl;
for (auto &gid : graph_ids)
  {
    dir.graphs[gid].load(matrix);
    sl.test(
      format("hill_climb %", dir.graphs[gid].filename), HillAlgo(matrix), 5);
    sl.test(
      format("annealing %", dir.graphs[gid].filename), AnneAlgo(matrix), 5);
    sl.dump(std::cout);
    std::cout << std::flush;
  }
  return 0;
}
