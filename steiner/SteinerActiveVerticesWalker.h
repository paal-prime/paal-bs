#ifndef STEINER_STEINERACTIVEVERTICESWALKER_H_
#define STEINER_STEINERACTIVEVERTICESWALKER_H_

#include <vector>
#include <cstring>
#include <set>
#include <utility>
#include <iterator>

#include "steiner/SteinerForest.h"
#include "steiner/SteinerForestUtils.h"
#include "steiner/SteinerForestInstance.h"
#include "graph/Graph.h"
#include "graph/AdjacencyMatrix.h"
#include "graph/AdjacencyLists.h"

namespace steiner
{
  /**
   * @brief [implements Walker] local search that in each iteration
   * slightly changes subgraph in which it computes minimum spanning tree.
   * @tparam G graph type.
   */
  template<typename G>
  class ActiveVerticesWalker
  {
    public:
      ActiveVerticesWalker(const G& graph, const int vertex_set[],
          std::vector<typename G::weighted_edge_t>& solution) : graph_(graph),
        current_solution_(solution)
      {
        vertices_count_ = graph.get_vertices_count();
        vertex_set_.reset(new int[vertices_count_]);
        memcpy(vertex_set_.get(), vertex_set,
               sizeof(vertex_set[0]) * vertices_count_);

        current_solution_points_.resize(vertices_count_, false);

        for (size_t i = 0; i < current_solution_.size(); ++i)
        {
          if (vertex_set_[current_solution_[i].target] == -1)
          {
            current_solution_points_[current_solution_[i].target] = true;
          }

          if (vertex_set_[current_solution_[i].source] == -1)
          {
            current_solution_points_[current_solution_[i].source] = true;
          }
        }

        current_fitness_ = fitness<G>(current_solution_);
      }

      template<typename Random> void prepare_step(double progress,
          Random &random)
      {
        next_solution_points_ = current_solution_points_;

        std::vector<size_t> points_active;
        std::vector<size_t> points_inactive;

        for (size_t i = 0; i < current_solution_points_.size(); ++i)
        {
          if (vertex_set_[i] == -1)
          {
            if (current_solution_points_[i])
            {
              points_active.push_back(i);
            }
            else
            {
              points_inactive.push_back(i);
            }
          }
        }

        int operation = random() % 3;

        if (!points_active.empty() && (operation == 0 || operation == 1))
        {
          next_solution_points_[points_active[random() % points_active.size()]]
            = false;
        }

        if (!points_inactive.empty() && (operation == 0 || operation == 2))
        {
          size_t point_to_activate =
            points_inactive[random() % points_inactive.size()];
          next_solution_points_[point_to_activate] = true;
        }

        next_solution_.clear();
        std::vector<bool> visited(vertices_count_, false);

        size_t seed;
        const size_t retry_limit = vertices_count_;
        size_t triesCount = 0;
        do
        {
          triesCount++;
          seed = random() % vertices_count_;
        }
        while (triesCount < retry_limit && !next_solution_points_[seed]
          && vertex_set_[seed] == -1);

        if (triesCount == retry_limit)
        {
          next_solution_ = current_solution_;
          next_fitness_ = current_fitness_;
          next_solution_points_ = current_solution_points_;
          return;
        }

        visited[seed] = true;

        auto it = graph_.out_edges(seed);

        std::multiset<typename G::weighted_edge_t> pool;

        while (it.first != it.second)
        {
          int v = (*it.first).target;
          if (next_solution_points_[v] || vertex_set_[v] != -1)
          {
            pool.insert(typename G::weighted_edge_t(seed, (*it.first).target,
                (*it.first).weight));
          }
          it.first++;
        }

        while (!pool.empty())
        {
          auto edge = pool.begin();

          if (!visited[edge->source] || !visited[edge->target])
          {
            int new_vertex;
            if (!visited[edge->source])
            {
              new_vertex = edge->source;
            }
            else
            {
              new_vertex = edge->target;
            }

            next_solution_.push_back(*edge);

            visited[new_vertex] = true;

            it = graph_.out_edges(new_vertex);
            while (it.first != it.second)
            {
              int v = (*it.first).target;
              if (next_solution_points_[v] || vertex_set_[v] != -1)
              {
                if (!visited[(*it.first).target])
                {
                  pool.insert(typename G::weighted_edge_t(new_vertex,
                      (*it.first).target, (*it.first).weight));
                }
              }
              it.first++;
            }
          }
          pool.erase(edge);
        }

        if (!is_feasible_solution(graph_, vertex_set_.get(), next_solution_))
        {
          next_solution_ = current_solution_;
          next_fitness_ = current_fitness_;
          next_solution_points_ = current_solution_points_;
          return;
        }

        next_solution_ = prune_solution<G>(graph_, vertex_set_.get(),
            next_solution_);
        next_fitness_ = fitness<G>(next_solution_);
      }

      void make_step()
      {
        current_solution_points_ = next_solution_points_;
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
      std::vector<bool> current_solution_points_;
      std::vector<bool> next_solution_points_;
      std::vector<typename G::weighted_edge_t> current_solution_;
      std::vector<typename G::weighted_edge_t> next_solution_;
  };
}  //  namespace steiner

#endif  // STEINER_STEINERACTIVEVERTICESWALKER_H_
