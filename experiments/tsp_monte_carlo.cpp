#include <iostream>
#include <random>

#include "paal/search.h"
#include "paal/StepCtrl.h"
#include "paal/ProgressCtrl.h"
#include "paal/SuperLogger.h"
#include "tsp/TwoOptWalker.h"
#include "tsp/MonteCarloAlgo.h"
#include "tsp/EuclidMatrix.h"
#include "tsp/TSPLIB.h"
#include "./format.h"

std::mt19937 random_(67628364);

typedef tsp::TSPLIB_Matrix Matrix;

struct Algo  // implements paal::Algo
{
  Algo(Matrix &_matrix) : matrix(_matrix) {}
  Matrix &matrix;

  template<typename Logger, typename StepCtrl, typename Cycle>
  double run_(Logger &logger, StepCtrl &step_ctrl, const Cycle &cycle) const
  {
    tsp::TwoOptWalker<Matrix> walker(matrix, cycle);
    //paal::IterationCtrl progress_ctrl(10000000);
    paal::TimeAutoCtrl progress_ctrl(15);
    //double begin = paal::realtime_sec();
    paal::search(walker, random_, progress_ctrl, step_ctrl, logger);
    //std::cout << format("fitness = %\n",tsp::fitness(matrix,walker.cycle));
    //for(int i=0; i<cycle.size(); ++i) std::cout << walker.cycle[i] << ' ';
    //std::cout << std::endl;
    std::cout << format("checkpoint_count = %\n", progress_ctrl.checkpoint_count);
    //double end = paal::realtime_sec();
    //std::cout << format("time=%\n",end-begin);
    std::cout << "it = " << logger.iterations << std::endl;
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


struct Greedy  // implements Algo
{
  Greedy(Matrix &_matrix) : matrix(_matrix) {}
  Matrix &matrix;
  template<typename Logger> double run(Logger &logger)
  {
    size_t n = matrix.size1();
    std::vector<size_t> cycle;
    bool V[matrix.size1()];
    for (size_t i = 0; i < matrix.size1(); ++i) V[i] = 0;
    cycle.push_back(0);
    V[0] = 1;
    for (size_t i = 1; i < n; ++i)
    {
      size_t bi = 0;
      for (size_t j = 0; j < n; ++j) if (!V[j] && (!bi ||
            matrix(cycle.back(), j) < matrix(cycle.back(), bi)))
          bi = j;
      //std::cout << bi << ' ';
      cycle.push_back(bi);
      V[bi] = 1;
    }
    //std::cout << std::endl;
    double fitness = tsp::fitness(matrix, cycle);
    logger.log(fitness);
    return fitness;
  }
};

int main()
{
  tsp::TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  std::vector<size_t> graph_ids = {9, 20, 21, 32, 65, 67, 91, 109};
  Matrix M;

  paal::SuperLogger sl;
for (size_t gid : graph_ids)
  {
    dir.graphs[gid].load(M);
    MonteCarlo<Matrix, std::mt19937, 0> MC0(M, random_, 15);
    MonteCarlo<Matrix, std::mt19937, 1> MC1(M, random_, 15);
    Greedy greedy(M);
    //M.generate(100,random_);
    sl.test(format("[case %] greedy", gid), greedy, 1);
    sl.test(format("[case %] MonteCarlo cheap", gid), MC0, 5);
    //sl.test(format("[case %] MonteCarlo expensive",gid),MC1,5);
    sl.test(format("[case %] HillClimb", gid), HillAlgo(M), 5);
    sl.dump(std::cout);
    std::cout << std::flush;
  }

  return 0;
}
