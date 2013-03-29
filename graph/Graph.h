#ifndef GRAPH_GRAPH_H_
#define GRAPH_GRAPH_H_

/**
 * Common classes/types of AdjacencyMatrix and AdjacencyLists.
 **/
namespace graph
{
  struct directed
  {
    enum { is_directed = true };
  };
  struct undirected
  {
    enum { is_directed = false };
  };
  struct unweighted {};

  template <typename V, typename W>
  class EdgeEnd
  {
    public:
      V target;
      W weight;
      EdgeEnd(V t, W w) : target(t), weight(w) {}
  };

  template <typename V>
  class Edge
  {
    public:
      Edge() : source(0), target(0) {}
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

  template <typename V, typename W>
  class WeightedEdge
  {
    public:
      WeightedEdge(V s, V t, W w) : source(s), target(t), weight(w) {}

      V source;
      V target;
      W weight;
  };

}  // namespace graph

#endif  // GRAPH_GRAPH_H_
