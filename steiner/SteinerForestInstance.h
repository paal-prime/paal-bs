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
 * @tparam V type representing vertex.
 * @tparam W codomain of weight/cost function.
 */
template <typename V = size_t, typename W = double>
class SteinerForestInstance
{
  public:
    explicit SteinerForestInstance(const std::string &input_file,
      const std::string &output_file)
    : input_file_(input_file), output_file_(output_file) {}

    /**
     * @brief reads data from files.
     * Must be run before using any getters.
     */
    void initialize()
    {
      std::ifstream input_data(input_file_);
      assert(input_data.is_open());
      input_data >> vertices_count_ >> edges_count_;

      vertex_set_.reset(new int[vertices_count_]);
      for (size_t i = 0; i < vertices_count_; ++i)
      {
        input_data >> vertex_set_[i];
      }

      for (size_t i = 0; i < edges_count_; ++i)
      {
        V a, b;
        W w;
        input_data >> a >> b >> w;
        edges_.push_back(graph::WeightedEdge<V, W>(a, b, w));
      }

      input_data.close();

      std::ifstream solution_data(output_file_);
      assert(solution_data.is_open());

      solution_data >> best_known_cost_;

      solution_data.close();
    }

    template <typename G>
    G get_graph()
    {
      G graph(vertices_count_);
      for (size_t i = 0; i < edges_.size(); ++i)
      {
        graph.add_edge(edges_[i].source, edges_[i].target,
          edges_[i].weight);
      }

      return graph;
    }

    const int* get_vertex_set()
    {
      return vertex_set_.get();
    }

    W get_best_known_cost()
    {
      return best_known_cost_;
    }
  private:
    size_t vertices_count_;
    size_t edges_count_;
    const std::string input_file_;
    const std::string output_file_;
    std::unique_ptr<int[]> vertex_set_;
    std::vector<graph::WeightedEdge<V, W> > edges_;
    W best_known_cost_;

};

#endif // STEINER_STEINERFORESTINSTANCE_H_
