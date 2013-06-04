#ifndef EXPERIMENTS_TSP_ALGO_H_
#define EXPERIMENTS_TSP_ALGO_H_

#include <random>

#include <paal/ProgressCtrl.h>
#include <mcts/MonteCarloTree.h>
#include <mcts/Policy.h>
#include <tsp/MCTS_tsp.h>
#include <tsp/TSPLIB.h>
#include "paal/search.h"
#include "paal/StepCtrl.h"
#include "tsp/TwoOptWalker.h"
#include "tsp/Christofides.h"
#include "tsp/util.h"

std::mt19937 random_(9823429);

typedef tsp::TSPLIB_Matrix Matrix;
typedef tsp::TSPState<Matrix> State;

/** @brief [implements Algo] */
struct LocalSearchAlgo
{
  LocalSearchAlgo(Matrix &_matrix, size_t _it) : matrix(_matrix), it(_it) {}
  Matrix &matrix;
  size_t it; //iterations

  template<typename Logger, typename StepCtrl, typename Cycle>
  double run_(Logger &logger, StepCtrl &step_ctrl, const Cycle &cycle) const
  {
    tsp::TwoOptWalker<Matrix> walker(matrix, cycle);
    paal::IterationCtrl progress_ctrl(it);
    //paal::TimeAutoCtrl progress_ctrl(15);
    //double begin = paal::realtime_sec();
    paal::search(walker, random_, progress_ctrl, step_ctrl, logger);
    //std::cout << "final cost: " << tsp::fitness(matrix,walker.cycle) << " " << walker.current_fitness() << std::endl;
    //double end = paal::realtime_sec();
    //std::cout << format("time=%\n",end-begin);
    return walker.current_fitness();
  }
};

struct HillAlgo : LocalSearchAlgo
{
  HillAlgo(Matrix &_matrix, size_t _it) : LocalSearchAlgo(_matrix, _it) {}

  template<typename Logger> double run(Logger &logger) const
  {
    std::vector<size_t> cycle;
    tsp::cycle_shuffle(cycle, matrix.size1(), random_);
    //std::cout << "initial cost: " << tsp::fitness(matrix,cycle) << std::endl;
    paal::HillClimb step_ctrl;
    return run_(logger, step_ctrl, cycle);
  }
};

struct HillTimeAlgo
{
  Matrix &matrix_;
  double time_limit_;
  HillTimeAlgo(Matrix &matrix, double time_limit) : matrix_(matrix), time_limit_(time_limit) {}

  template<typename Logger> double run(Logger &logger) const
  {
    std::vector<size_t> cycle;
    tsp::cycle_shuffle(cycle, matrix_.size1(), random_);
    tsp::TwoOptWalker<Matrix> walker(matrix_, cycle);
    paal::TimeAutoCtrl progress_ctrl(time_limit_);
    paal::HillClimb step_ctrl;
    paal::search(walker, random_, progress_ctrl, step_ctrl, logger);
    return walker.current_fitness();
  }
};

struct AnneAlgo : LocalSearchAlgo
{
  AnneAlgo(Matrix &_matrix, size_t _it, double _t1) :
    LocalSearchAlgo(_matrix, _it), t1(_t1) {}
  double t1;

  template<typename Logger> double run(Logger &logger) const
  {
    std::vector<size_t> cycle;
    tsp::cycle_shuffle(cycle, matrix.size1(), random_);
    paal::Annealing step_ctrl(
      tsp::fitness(matrix, cycle) / matrix.size1(), t1);
    return run_(logger, step_ctrl, cycle);
  }
};

/** @brief [implements Algo] */
template<typename Policy> struct MCTSAlgo
{
  State *state_;
  Policy policy_;
  size_t full_search_;
  double samples_ratio_;

  MCTSAlgo(Policy policy, size_t samples_ratio = 500, size_t full_search = 9) :
    state_(NULL), policy_(policy), full_search_(full_search),
    samples_ratio_(samples_ratio) {}

  template<typename Logger> double run(Logger &logger) const
  {
    mcts::MonteCarloTree<State, Policy> mct(*state_, policy_);
    while (!mct.root_state().is_terminal())
    {
      if (mct.root_state().left_decisions() > full_search_)
      {
        size_t samples = mct.root_state().left_decisions() * samples_ratio_;
        paal::IterationCtrl ctrl(samples);
        auto move = mct.search(ctrl);
        mct.apply(move);
      }
      else
      {
        mct.root_state().exhaustive_search_min();
      }
    }
    return mct.root_state().cost_;
  }
};

struct ChristofidesAlgo
{
  std::string ewt;
  Matrix &mtx;

  ChristofidesAlgo(Matrix &_mtx, std::string _ewt = "") : ewt(_ewt), mtx(_mtx) {}

  template<typename Logger> double run(Logger &logger) const
  {
    std::vector<int> cycle;
    cycle.resize(mtx.size1());
    if (ewt == "EUC_2D" || ewt == "CEIL_2D" || ewt == "ATT") {
      tsp::christofides<Matrix, std::vector<int> >(mtx, cycle, mtx.size1(), ewt,
          &mtx.pos);
    } else {
      tsp::christofides<Matrix, std::vector<int> >(mtx, cycle, mtx.size1());
    }
    return tsp::fitness(mtx, cycle);
  }
};

template<typename Policy> MCTSAlgo<Policy> make_algo(Policy policy)
{
  return MCTSAlgo<Policy>(policy);
}

#endif  // EXPERIMENTS_TSP_ALGO_H_
