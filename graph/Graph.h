#ifndef GRAPH_GRAPH_H_
#define GRAPH_GRAPH_H_

/**
 * Common classes/types of AdjacencyMatrix and AdjacencyLists.
 **/
namespace graph
{
  /**
   * @brief struct used as template argument to create directed
   * graph.
   */
  struct directed
  {
    enum { is_directed = true };
  };


  /**
   * @brief struct used as template argument to create undirected
   * graph.
   */
  struct undirected
  {
    enum { is_directed = false };
  };

  /**
   * @brief struct used as template argument to create unweighted
   * graph.
   */
  struct unweighted {};

  /**
   * @brief struct containing edge's weight and neighbouring vertex
   * index. Used when iterating over vertex's edges.
   * @tparam V vertices index type.
   * @tparam W edge weight type.
   */
  template <typename V, typename W>
  class EdgeEnd
  {
    public:
      V target;
      W weight;
      EdgeEnd(V t, W w) : target(t), weight(w) {}
  };

  /**
   * @brief representation of edge in unweighted graph.
   * @tparam V vertices index type.
   */
  template <typename V>
  class Edge
  {
    public:
      Edge() : source(V()), target(V()) {}
      Edge(V s, V t) : source(s), target(t) {}

      bool operator < (const Edge& rhs) const
      {
        if (source == rhs.source)
        {
          return target < rhs.target;
        }
        else
        {
          return source < rhs.source;
        }
      }

      V source;
      V target;
  };

  /**
   * @brief representation of edge in weighted graph.
   * @tparam V vertices index type.
   * @tparam W edge weight type.
   */
  template <typename V, typename W>
  class WeightedEdge
  {
    public:
      WeightedEdge(V s, V t, W w) : source(s), target(t), weight(w) {}

      bool operator < (const WeightedEdge& rhs) const
      {
        if (weight == rhs.weight)
        {
          if (source == rhs.source)
          {
            return target < rhs.target;
          }
          else
          {
            return source < rhs.source;
          }
        }
        else
        {
          return weight < rhs.weight;
        }
      }

      V source;
      V target;
      W weight;
  };

}  // namespace graph

#endif  // GRAPH_GRAPH_H_
