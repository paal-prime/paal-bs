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
}  // namespace Graph

#endif  // GRAPH_GRAPH_H_
