#ifndef GRAPH_GRAPH_H_
#define GRAPH_GRAPH_H_

namespace Graph
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
        if (this->source == rhs.source)
        {
          return this->target < rhs.source;
        }
        else
        {
          return this->source < rhs.source;
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

}  // namespace Graph

#endif  // GRAPH_GRAPH_H_
