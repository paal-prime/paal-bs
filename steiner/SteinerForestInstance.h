#ifndef STEINER_STEINERFORESTINSTANCE_H_
#define STEINER_STEINERFORESTINSTANCE_H_

#include <cassert>
#include <string>
#include <memory>
#include <fstream>  // NOLINT
#include <vector>
#include "graph/Graph.h"

/**
 * @brief represents SteinerForest test case instance.
 * @tparam G graph type.
 */
template <typename G>
class SteinerForestInstance
{
  public:
    explicit SteinerForestInstance()
    : graph_(0) {}

    /**
     * @brief reads data from files.
     * Must be run before using any getters.
     */
    void load(const std::string &input_file,
      const std::string &solution_file = "")
    {
      std::ifstream input_data(input_file);
      assert(input_data.is_open());
      size_t vertices_count;
      size_t edges_count;
      input_data >> vertices_count >> edges_count;

      graph_.reset(vertices_count);

      vertex_set_.reset(new int[vertices_count]);
      for (size_t i = 0; i < vertices_count; ++i)
      {
        input_data >> vertex_set_[i];
      }

      for (size_t i = 0; i < edges_count; ++i)
      {
        typename G::vertex_t a, b;
        typename G::edge_weight_t w;
        input_data >> a >> b >> w;
        graph_.add_edge(a, b, w);
      }

      input_data.close();

      if(solution_file != "")
      {
        std::ifstream solution_data(solution_file);
        assert(solution_data.is_open());

        solution_data >> best_known_cost_;

        solution_data.close();
      }
      else
      {
        best_known_cost_ = -1;
      }
    }

    const G& get_graph()
    {
      return graph_;
    }

    const int* get_vertex_set()
    {
      return vertex_set_.get();
    }

    typename G::edge_weight_t get_best_known_cost()
    {
      return best_known_cost_;
    }
  private:
    G graph_;
    std::unique_ptr<int[]> vertex_set_;
    typename G::edge_weight_t best_known_cost_;

};

#endif // STEINER_STEINERFORESTINSTANCE_H_
