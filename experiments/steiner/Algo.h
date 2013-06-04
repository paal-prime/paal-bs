#ifndef EXPERIMENTS_STEINER_ALGO_H_
#define EXPERIMENTS_STEINER_ALGO_H_

#include <random>
#include <cstring>

#include "graph/Graph.h"
#include "graph/AdjacencyMatrix.h"
#include "graph/AdjacencyLists.h"
#include "steiner/SteinerBreakCycleWalker.h"
#include "steiner/SteinerActiveVerticesWalker.h"

#include "paal/search.h"
#include "paal/ProgressCtrl.h"
#include "paal/StepCtrl.h"
#include "paal/SuperLogger.h"

std::mt19937 random_(786284);

/** @brief [implements Algo] */
template <typename GRAPH>
struct SFApxAlgo
{
  typedef GRAPH graph_t;
  SFApxAlgo(const graph_t& _graph, const int _vertex_set[]) : graph(_graph), vertex_set(_vertex_set)
  {
  }

  const graph_t &graph;
  const int * const vertex_set;

  template<typename Logger>
  double run(Logger &logger) const
  {
    std::vector<typename graph_t::weighted_edge_t> steiner_forest_edges;
    steiner::steiner_forest<>(graph, vertex_set, steiner_forest_edges);

    return steiner::fitness<graph_t>(steiner_forest_edges);
  }
};

template <typename GRAPH, typename WALKER>
struct SFLocalSearchAlgo
{
  typedef GRAPH graph_t;
  SFLocalSearchAlgo(const graph_t& _graph, const int _vertex_set[],
    std::vector<typename graph_t::weighted_edge_t>& _initial_solution, int _iterations)
  : graph(_graph), vertex_set(_vertex_set), initial_solution(_initial_solution), iterations(_iterations)
  {
  }
  const graph_t &graph;
  const int * const vertex_set;
  std::vector<typename graph_t::weighted_edge_t> &initial_solution;
  const int iterations;

  template<typename Logger, typename StepCtrl>
  double run_(Logger &logger, StepCtrl &step_ctrl) const
  {
    WALKER walker(graph, vertex_set, initial_solution);
    paal::IterationCtrl progress_ctrl(iterations);
    paal::search(walker, random_, progress_ctrl, step_ctrl, logger);
    return walker.current_fitness();
  }
};

/** @brief [implements Algo] */
template <typename GRAPH, typename WALKER>
struct SFHillAlgo : SFLocalSearchAlgo<GRAPH, WALKER>
{
  typedef GRAPH graph_t;
  SFHillAlgo(const graph_t& _graph, const int _vertex_set[],
    std::vector<typename graph_t::weighted_edge_t> &_initial_solution,
    int _iterations)
  : SFLocalSearchAlgo<GRAPH, WALKER>(_graph, _vertex_set, _initial_solution, _iterations) {}

  template<typename Logger> double run(Logger &logger) const
  {
    paal::HillClimb step_ctrl;
    return SFLocalSearchAlgo<GRAPH, WALKER>::run_(logger, step_ctrl);
  }
};

/** @brief [implements Algo] */
template <typename GRAPH, typename WALKER>
struct SFAnneAlgo : SFLocalSearchAlgo<GRAPH, WALKER>
{
  typedef GRAPH graph_t;
  SFAnneAlgo(const graph_t& _graph, const int _vertex_set[],
    std::vector<typename graph_t::weighted_edge_t> &_initial_solution,
    int _iterations, double _t1)
  : SFLocalSearchAlgo<GRAPH, WALKER>(_graph, _vertex_set, _initial_solution, _iterations), t1(_t1)
  {
  }
  double t1;

  template<typename Logger> double run(Logger &logger) const
  {
    paal::Annealing step_ctrl(
      steiner::fitness<graph_t>(SFLocalSearchAlgo<GRAPH, WALKER>::initial_solution) / SFLocalSearchAlgo<GRAPH, WALKER>::initial_solution.size(), t1);
    return SFLocalSearchAlgo<GRAPH, WALKER>::run_(logger, step_ctrl);
  }
};

#endif  // EXPERIMENTS_STEINER_ALGO_H_
