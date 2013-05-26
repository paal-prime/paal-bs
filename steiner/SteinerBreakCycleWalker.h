#ifndef STEINER_STEINERBREAKCYCLEWALKER_H_
#define STEINER_STEINERBREAKCYCLEWALKER_H_

#include <vector>
#include <cstring>
#include <set>
#include <utility>
#include <iterator>
#include <algorithm>

#include "steiner/SteinerForest.h"
#include "steiner/SteinerForestUtils.h"
#include "steiner/SteinerForestInstance.h"
#include "graph/Graph.h"
#include "graph/AdjacencyMatrix.h"
#include "graph/AdjacencyLists.h"

namespace steiner
{
  /**
   * @brief [implements Walker] break cycle local search.
   */
  template<typename G>
  class BreakCycleWalker
  {
    public:
      BreakCycleWalker(const G& graph, const int vertex_set[],
          std::vector<typename G::weighted_edge_t>& solution) : graph_(graph),
        current_solution_(solution)
      {
        vertices_count_ = graph.get_vertices_count();
        vertex_set_.reset(new int[vertices_count_]);
        memcpy(vertex_set_.get(), vertex_set,
               sizeof(vertex_set[0]) * vertices_count_);
        current_fitness_ = fitness<G>(current_solution_);
      }

      /**
       * @brief finds any nontrivial cycle in graph starting in vertex v.
       * @param v starting vertex.
       * @param graph input graph.
       * @param visited array storing information which vertices where
       * @param parent DFS parent array.
       * already visited.
       * @param cycle found cycle's edges.
       * @tparam GRAPH graph type.
       */
      template<typename GRAPH>
      bool find_cycle(size_t v, const GRAPH& graph, std::vector<bool>& visited,
          std::vector<int>& parent,
          std::vector<typename G::weighted_edge_t>& cycle)
      {
        visited[v] = true;

        auto it = graph.out_edges(v);

        while (it.first != it.second)
        {
          size_t neighbour = (*it.first).target;
          if (!visited[neighbour])
          {
            parent[neighbour] = v;
            cycle.push_back(typename G::weighted_edge_t(v, (*it.first).target,
                (*it.first).weight));
            if (find_cycle(neighbour, graph, visited, parent, cycle))
            {
              return true;
            }
            else
            {
              cycle.pop_back();
            }
          }
          else
          {
            if (parent[v] != static_cast<int>(neighbour))
            {
              return true;
            }
          }

          it.first++;
        }

        return false;
      }

      /**
       * @brief checks whethere given solution contains edge (a,b).
       */
      bool isIn(size_t a, size_t b,
          std::vector<typename G::weighted_edge_t>& solution)
      {
        for (size_t i = 0; i < solution.size(); ++i)
        {
          if ((solution[i].source == a && solution[i].target == b)
              || (solution[i].source == b && solution[i].target == a))
          {
            return true;
          }
        }

        return false;
      }

      /**
       * @brief search for a cycle in solution, if cycle is found,
       * remove the heaviest edge of the cycle.
       */
      void try_to_remove_edge(
        std::vector<typename G::weighted_edge_t>& solution, size_t v)
      {
        graph::AdjacencyLists<graph::undirected, int> graph(vertices_count_);

        for (size_t i = 0; i < solution.size(); ++i)
        {
          graph.add_edge(solution[i].source, solution[i].target,
              solution[i].weight);
        }

        std::vector<bool> visited(vertices_count_, false);
        std::vector<int> parent(vertices_count_, -1);
        std::vector<typename G::weighted_edge_t> cycle;

        if (!find_cycle(v, graph, visited, parent, cycle))
        {
          return;
        }

        size_t edge_index = 0;

        for (size_t i = 1; i < cycle.size(); ++i)
        {
          if (cycle[i].weight > cycle[edge_index].weight)
          {
            edge_index = i;
          }
        }

        for (size_t i = 0; i < solution.size(); ++i)
        {
          if (std::min(cycle[edge_index].source, cycle[edge_index].target)
              == std::min(solution[i].source, solution[i].target)
              &&
              std::max(cycle[edge_index].source, cycle[edge_index].target)
              == std::max(solution[i].source, solution[i].target))
          {
            std::swap(solution[i], solution.back());
            solution.pop_back();
            break;
          }
        }
      }

      template<typename Random> void prepare_step(double progress,
          Random &random)
      {
        next_solution_ = current_solution_;

        size_t a;
        size_t b;

        std::vector<bool> solution_vertex(vertices_count_, false);

        for (size_t i = 0; i < current_solution_.size(); ++i)
        {
          solution_vertex[current_solution_[i].source] = true;
          solution_vertex[current_solution_[i].target] = true;
        }

        do
        {
          a = random() % vertices_count_;
          b = random() % vertices_count_;
        }
        while (a == b || !solution_vertex[a] || !solution_vertex[b]
               || !graph_.adjacent(a, b) || isIn(a, b, next_solution_));

        next_solution_.push_back(typename G::weighted_edge_t(a, b,
            graph_.edge(a, b).second));
        try_to_remove_edge(next_solution_, a);
        next_solution_ = prune_solution(graph_, vertex_set_.get(),
            next_solution_);
        next_fitness_ = fitness<G>(next_solution_);
      }

      void make_step()
      {
        current_solution_ = next_solution_;
        current_fitness_ = next_fitness_;
      }

      double current_fitness()
      {
        return current_fitness_;
      }

      double next_fitness()
      {
        return next_fitness_;
      }

    private:
      G graph_;
      std::unique_ptr<int[]> vertex_set_;
      size_t vertices_count_;
      double current_fitness_;
      double next_fitness_;
      std::vector<typename G::weighted_edge_t> current_solution_;
      std::vector<typename G::weighted_edge_t> next_solution_;
  };
}  //  namespace steiner

#endif  // STEINER_STEINERBREAKCYCLEWALKER_H_
