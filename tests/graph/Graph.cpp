#include <gtest/gtest.h>
#include <utility>
#include <vector>
#include <set>
#include <map>
#include "graph/AdjacencyMatrix.h"
#include "graph/AdjacencyLists.h"

class EdgeEnd
{
  public:
    explicit EdgeEnd(size_t t) : target(t) {}
    size_t target;
};

class WeightedEdgeEnd
{
  public:
    WeightedEdgeEnd(size_t t, int w) : target(t), weight(w) {}
    size_t target;
    int weight;
};

enum directed_t
{
  DIRECTED,
  UNDIRECTED
};

enum weighted_t
{
  WEIGHTED,
  UNWEIGHTED
};

template <typename E>
class GraphParam
{
  public:
    GraphParam(size_t vertices_count, directed_t is_directed,
        weighted_t is_weighted, std::vector< std::vector <E> > adj_list)
      : vertices_count_(vertices_count), is_directed_(is_directed),
        is_weighted_(is_weighted), adj_list_(adj_list) {}

    size_t vertices_count_;
    directed_t is_directed_;
    weighted_t is_weighted_;
    std::vector< std::vector<E> > adj_list_;
};

class UnweightedGraphTest: public ::testing::TestWithParam
  < GraphParam<EdgeEnd> >
{
};

class WeightedGraphTest: public ::testing::TestWithParam
  < GraphParam<WeightedEdgeEnd> >
{
};

template <typename G, typename P>
void init_unweighted_graph_test(G& graph, const P& param,
    std::set< std::pair<size_t, size_t> >& edges, std::map < size_t,
    std::set<size_t> >& adjacent_vertices)
{
  for (size_t source = 0; source < param.adj_list_.size(); ++source)
  {
    for (size_t neighbour = 0; neighbour < param.adj_list_[source].size();
         ++neighbour)
    {
      size_t target = param.adj_list_[source][neighbour].target;

      edges.insert(std::make_pair(source, target));
      adjacent_vertices[source].insert(target);
      if (param.is_directed_ == UNDIRECTED)
      {
        edges.insert(std::make_pair(target, source));
        adjacent_vertices[target].insert(source);
      }

      graph.add_edge(source, target);
    }
  }
}


template <typename G, typename P>
void init_weighted_graph_test(G& graph, const P& param,
    std::set< std::pair<size_t, size_t> >& edges,
    std::map<size_t, std::set<size_t> >& adjacent_vertices,
    std::map< std::pair<size_t, size_t>, int >& edges_weights)
{
  for (size_t source = 0; source < param.adj_list_.size(); ++source)
  {
    for (size_t neighbour = 0; neighbour < param.adj_list_[source].size();
         ++neighbour)
    {
      size_t target = param.adj_list_[source][neighbour].target;
      int weight = param.adj_list_[source][neighbour].weight;

      edges.insert(std::make_pair(source, target));
      edges_weights[std::make_pair(source, target)] = weight;
      adjacent_vertices[source].insert(target);
      if (param.is_directed_ == UNDIRECTED)
      {
        edges.insert(std::make_pair(target, source));
        edges_weights[std::make_pair(target, source)] = weight;
        adjacent_vertices[target].insert(source);
      }

      graph.add_edge(source, target, weight);
    }
  }
}

template <typename G, typename P>
void test_unweighted_graph(const G& graph, const P& param,
    std::set< std::pair<size_t, size_t> >& edges,
    std::map<size_t, std::set<size_t> >& adjacent_vertices)
{
  typedef G graph_t;
  typedef typename graph_t::vertex_t vertex_t;
  typedef typename graph_t::edge_weight_t edge_weight_t;
  typedef typename graph_t::edge_iterator_t edge_iterator_t;
  typedef typename graph_t::adjacency_iterator_t adjacency_iterator_t;
  typedef typename graph_t::edge_t edge_t;
  typedef typename graph_t::weighted_edge_t weighted_edge_t;

  for (size_t source = 0; source < param.vertices_count_; ++source)
  {
    for (size_t target = 0; target < param.vertices_count_; ++target)
    {
      bool adjacent = edges.count(std::make_pair(source, target)) > 0;
      ASSERT_EQ(graph.adjacent(source, target), adjacent);
    }
  }

  for (size_t source = 0; source < param.vertices_count_; ++source)
  {
    std::pair<adjacency_iterator_t, adjacency_iterator_t> it =
      graph.adjacent_vertices(source);

    std::set<size_t> neighbours;

    while (it.first != it.second)
    {
      neighbours.insert(*(it.first));
      it.first++;
    }

    ASSERT_EQ(neighbours, adjacent_vertices[source]);
  }
}


template <typename G, typename P>

void test_weighted_graph(const G& graph, const P& param,
    std::set< std::pair<size_t, size_t> >& edges,
    std::map<size_t, std::set<size_t> >& adjacent_vertices,
    std::map< std::pair<size_t, size_t>, int >& edges_weights)
{
  typedef typename G::edge_weight_t edge_weight_t;
  typedef typename G::edge_iterator_t edge_iterator_t;

  test_unweighted_graph(graph, param, edges, adjacent_vertices);

  for (size_t source = 0; source < param.vertices_count_; ++source)
  {
    for (size_t target = 0; target < param.vertices_count_; ++target)
    {
      std::pair<bool, edge_weight_t> result = graph.edge(source, target);
      bool is_edge = edges.count(std::make_pair(source, target)) > 0;
      ASSERT_EQ(result.first, is_edge);
      if (is_edge)
      {
        ASSERT_EQ(result.second, edges_weights[std::make_pair(source, target)]);
      }
    }
  }

  for (size_t source = 0; source < param.vertices_count_; ++source)
  {
    std::set< std::pair<size_t, int> > out_edges;
    for (size_t target = 0; target < param.vertices_count_; ++target)
    {
      if (edges.count(std::make_pair(source, target)) > 0)
      {
        int weight = edges_weights[std::make_pair(source, target)];
        out_edges.insert(std::pair<size_t, int>(target, weight));
      }
    }

    std::pair<edge_iterator_t, edge_iterator_t> it = graph.out_edges(source);

    std::set< std::pair<size_t, int> > edge_ends;
    while (it.first != it.second)
    {
      edge_ends.insert(std::pair<size_t, int>((*it.first).target,
          (*it.first).weight));
      it.first++;
    }

    ASSERT_EQ(edge_ends, out_edges);
  }
}

TEST_P(UnweightedGraphTest, AdjacencyMatrix)
{
  GraphParam<EdgeEnd> param = GetParam();

  std::set< std::pair<size_t, size_t> > edges;
  std::map<size_t, std::set<size_t> > adjacent_vertices;

  if (param.is_directed_)
  {
    typedef AdjacencyMatrix<graph::undirected, graph::unweighted> graph_t;
    graph_t graph(param.vertices_count_);
    ASSERT_EQ(graph.get_vertices_count(), param.vertices_count_);
    init_unweighted_graph_test(graph, param, edges, adjacent_vertices);
    test_unweighted_graph(graph, param, edges, adjacent_vertices);
  }
  else
  {
    typedef AdjacencyMatrix<graph::directed, graph::unweighted> graph_t;
    graph_t graph(param.vertices_count_);
    ASSERT_EQ(graph.get_vertices_count(), param.vertices_count_);
    init_unweighted_graph_test(graph, param, edges, adjacent_vertices);
    test_unweighted_graph(graph, param, edges, adjacent_vertices);
  }
}

TEST_P(UnweightedGraphTest, AdjacencyLists)
{
  GraphParam<EdgeEnd> param = GetParam();

  std::set< std::pair<size_t, size_t> > edges;
  std::map<size_t, std::set<size_t> > adjacent_vertices;

  if (param.is_directed_)
  {
    typedef AdjacencyLists<graph::undirected, graph::unweighted> graph_t;
    graph_t graph(param.vertices_count_);
    ASSERT_EQ(graph.get_vertices_count(), param.vertices_count_);
    init_unweighted_graph_test(graph, param, edges, adjacent_vertices);
    test_unweighted_graph(graph, param, edges, adjacent_vertices);
  }
  else
  {
    typedef AdjacencyLists<graph::directed, graph::unweighted> graph_t;
    graph_t graph(param.vertices_count_);
    ASSERT_EQ(graph.get_vertices_count(), param.vertices_count_);
    init_unweighted_graph_test(graph, param, edges, adjacent_vertices);
    test_unweighted_graph(graph, param, edges, adjacent_vertices);
  }
}

TEST_P(WeightedGraphTest, AdjacencyMatrix)
{
  GraphParam<WeightedEdgeEnd> param = GetParam();

  std::set< std::pair<size_t, size_t> > edges;
  std::map<size_t, std::set<size_t> > adjacent_vertices;
  std::map< std::pair<size_t, size_t>, int > edges_weights;

  if (param.is_directed_)
  {
    typedef AdjacencyMatrix<graph::undirected, int> graph_t;
    graph_t graph(param.vertices_count_);
    ASSERT_EQ(graph.get_vertices_count(), param.vertices_count_);
    init_weighted_graph_test(graph, param, edges, adjacent_vertices,
        edges_weights);
    test_weighted_graph(graph, param, edges, adjacent_vertices, edges_weights);
  }
  else
  {
    typedef AdjacencyMatrix<graph::directed, int> graph_t;
    graph_t graph(param.vertices_count_);
    ASSERT_EQ(graph.get_vertices_count(), param.vertices_count_);
    init_weighted_graph_test(graph, param, edges, adjacent_vertices,
        edges_weights);
    test_weighted_graph(graph, param, edges, adjacent_vertices, edges_weights);
  }
}


TEST_P(WeightedGraphTest, AdjacencyLists)
{
  GraphParam<WeightedEdgeEnd> param = GetParam();

  std::set< std::pair<size_t, size_t> > edges;
  std::map<size_t, std::set<size_t> > adjacent_vertices;
  std::map< std::pair<size_t, size_t>, int > edges_weights;


  if (param.is_directed_)
  {
    typedef AdjacencyLists<graph::undirected, int> graph_t;
    graph_t graph(param.vertices_count_);
    ASSERT_EQ(graph.get_vertices_count(), param.vertices_count_);
    init_weighted_graph_test(graph, param, edges, adjacent_vertices,
        edges_weights);
    test_weighted_graph(graph, param, edges, adjacent_vertices, edges_weights);
  }
  else
  {
    typedef AdjacencyLists<graph::directed, int> graph_t;
    graph_t graph(param.vertices_count_);
    ASSERT_EQ(graph.get_vertices_count(), param.vertices_count_);
    init_weighted_graph_test(graph, param, edges, adjacent_vertices,
        edges_weights);
    test_weighted_graph(graph, param, edges, adjacent_vertices, edges_weights);
  }
}

//  Reference http://en.wikipedia.org/wiki/Gallery_of_named_graphs
INSTANTIATE_TEST_CASE_P(SmallGraphs, UnweightedGraphTest,
    ::testing::Values(
      //  Diamond graph
      GraphParam<EdgeEnd>(4, UNDIRECTED, UNWEIGHTED,
{
  {EdgeEnd(1)},
  {EdgeEnd(2)},
  {EdgeEnd(0), EdgeEnd(3)},
  {EdgeEnd(0)}
}),
      //  Wagner graph
      GraphParam<EdgeEnd>(8, UNDIRECTED, UNWEIGHTED,
{
  {EdgeEnd(4)},
  {EdgeEnd(5)},
  {EdgeEnd(6)},
  {EdgeEnd(7)}
}),
      //  Bull graph
      GraphParam<EdgeEnd>(5, UNDIRECTED, UNWEIGHTED,
{
  {EdgeEnd(2)},
  {EdgeEnd(3)},
  {EdgeEnd(3), EdgeEnd(4)},
  {EdgeEnd(4)}
}),
      //  Butterfly graph
      GraphParam<EdgeEnd>(5, UNDIRECTED, UNWEIGHTED,
{
  {EdgeEnd(1)},
  {EdgeEnd(2)},
  {EdgeEnd(0), EdgeEnd(3)},
  {EdgeEnd(4)},
  {EdgeEnd(2)}
}),
      //  Moser spindle
      GraphParam<EdgeEnd>(7, UNDIRECTED, UNWEIGHTED,
{
  {EdgeEnd(1), EdgeEnd(2)},
  {EdgeEnd(2), EdgeEnd(5)},
  {EdgeEnd(5)},
  {EdgeEnd(0), EdgeEnd(4), EdgeEnd(6)},
  {EdgeEnd(0), EdgeEnd(6)},
  {EdgeEnd(6)}
}),
      //  Diamond graph
      GraphParam<EdgeEnd>(4, DIRECTED, UNWEIGHTED,
{
  {EdgeEnd(1)},
  {EdgeEnd(2)},
  {EdgeEnd(0), EdgeEnd(3)},
  {EdgeEnd(0)}
}),
      //  Wagner graph
      GraphParam<EdgeEnd>(8, DIRECTED, UNWEIGHTED,
{
  {EdgeEnd(4)},
  {EdgeEnd(5)},
  {EdgeEnd(6)},
  {EdgeEnd(7)},
  {EdgeEnd(0)},
  {EdgeEnd(1)},
  {EdgeEnd(2)},
  {EdgeEnd(3)}
}),
      //  Bull graph
      GraphParam<EdgeEnd>(5, DIRECTED, UNWEIGHTED,
{
  {EdgeEnd(2)},
  {EdgeEnd(3)},
  {EdgeEnd(3), EdgeEnd(4)},
  {EdgeEnd(4)}
}),
      //  Butterfly graph
      GraphParam<EdgeEnd>(5, DIRECTED, UNWEIGHTED,
{
  {EdgeEnd(1)},
  {EdgeEnd(2)},
  {EdgeEnd(0), EdgeEnd(3)},
  {EdgeEnd(4)},
  {EdgeEnd(2)}
}),
      //  Moser spindle
      GraphParam<EdgeEnd>(7, DIRECTED, UNWEIGHTED,
{
  {EdgeEnd(1), EdgeEnd(2)},
  {EdgeEnd(0), EdgeEnd(2), EdgeEnd(5)},
  {EdgeEnd(0), EdgeEnd(5), EdgeEnd(1)},
  {EdgeEnd(0), EdgeEnd(4), EdgeEnd(6)},
  {EdgeEnd(0), EdgeEnd(6)},
  {EdgeEnd(2), EdgeEnd(1), EdgeEnd(6)},
  {EdgeEnd(4), EdgeEnd(5)}
})));

INSTANTIATE_TEST_CASE_P(SmallGraphs, WeightedGraphTest,
    ::testing::Values(
      //  Diamond graph
      GraphParam<WeightedEdgeEnd>(4, UNDIRECTED, WEIGHTED,
{
  {WeightedEdgeEnd(1, 2)},
  {WeightedEdgeEnd(2, 3)},
  {WeightedEdgeEnd(0, 42), WeightedEdgeEnd(3, 7)},
  {WeightedEdgeEnd(0, 1)}
}),
      //  Wagner graph
      GraphParam<WeightedEdgeEnd>(8, UNDIRECTED, WEIGHTED,
{
  {WeightedEdgeEnd(4, 1)},
  {WeightedEdgeEnd(5, 2)},
  {WeightedEdgeEnd(6, 3)},
  {WeightedEdgeEnd(7, 4)}
}),
      //  Bull graph
      GraphParam<WeightedEdgeEnd>(5, UNDIRECTED, WEIGHTED,
{
  {WeightedEdgeEnd(2, -1)},
  {WeightedEdgeEnd(3, 4)},
  {WeightedEdgeEnd(3, 7), WeightedEdgeEnd(4, 16)},
  {WeightedEdgeEnd(4, -9)}
}),
      //  Butterfly graph
      GraphParam<WeightedEdgeEnd>(5, UNDIRECTED, WEIGHTED,
{
  {WeightedEdgeEnd(1, 124)},
  {WeightedEdgeEnd(2, 1244)},
  {WeightedEdgeEnd(0, -421), WeightedEdgeEnd(3, 4589)},
  {WeightedEdgeEnd(4, 190)},
  {WeightedEdgeEnd(2, 5348902)}
}),
      //  Moser spindle
      GraphParam<WeightedEdgeEnd>(7, UNDIRECTED, WEIGHTED,
{
  {WeightedEdgeEnd(1, 345890), WeightedEdgeEnd(2, -1239)},
  {WeightedEdgeEnd(2, -34589), WeightedEdgeEnd(5, 123490)},
  {WeightedEdgeEnd(5, 234)},
  {
    WeightedEdgeEnd(0, 90345), WeightedEdgeEnd(4, 12390),
    WeightedEdgeEnd(6, 890)
  },
  {WeightedEdgeEnd(0, 12489), WeightedEdgeEnd(6, -890238)},
  {WeightedEdgeEnd(6, -2319)}
}),
      //  Diamond graph
      GraphParam<WeightedEdgeEnd>(4, DIRECTED, WEIGHTED,
{
  {WeightedEdgeEnd(1, 2)},
  {WeightedEdgeEnd(2, 3)},
  {WeightedEdgeEnd(0, 42), WeightedEdgeEnd(3, 7)},
  {WeightedEdgeEnd(0, 1)}
}),
      //  Wagner graph
      GraphParam<WeightedEdgeEnd>(8, DIRECTED, WEIGHTED,
{
  {WeightedEdgeEnd(4, 1)},
  {WeightedEdgeEnd(5, 2)},
  {WeightedEdgeEnd(6, 3)},
  {WeightedEdgeEnd(7, 4)},
  {WeightedEdgeEnd(0, 5)},
  {WeightedEdgeEnd(1, 2)},
  {WeightedEdgeEnd(2, 2)},
  {WeightedEdgeEnd(3, 7)}
}),
      //  Bull graph
      GraphParam<WeightedEdgeEnd>(5, DIRECTED, WEIGHTED,
{
  {WeightedEdgeEnd(2, -1)},
  {WeightedEdgeEnd(3, 4)},
  {WeightedEdgeEnd(3, 7), WeightedEdgeEnd(4, 16)},
  {WeightedEdgeEnd(4, -9)}
}),
      //  Butterfly graph
      GraphParam<WeightedEdgeEnd>(5, DIRECTED, WEIGHTED,
{
  {WeightedEdgeEnd(1, 124)},
  {WeightedEdgeEnd(2, 1244)},
  {WeightedEdgeEnd(0, -421), WeightedEdgeEnd(3, 4589)},
  {WeightedEdgeEnd(4, 190)},
  {WeightedEdgeEnd(2, 5348902)}
}),
      //  Moser spindle
      GraphParam<WeightedEdgeEnd>(7, DIRECTED, WEIGHTED,
{
  {WeightedEdgeEnd(1, 345890), WeightedEdgeEnd(2, -1239)},
  {
    WeightedEdgeEnd(0, 12390), WeightedEdgeEnd(2, -34589),
    WeightedEdgeEnd(5, 123490)
  },
  {WeightedEdgeEnd(0, -2349), WeightedEdgeEnd(5, 234), WeightedEdgeEnd(1, 456)},
  {
    WeightedEdgeEnd(0, 90345), WeightedEdgeEnd(4, 12390),
    WeightedEdgeEnd(6, 890)
  },
  {WeightedEdgeEnd(0, 12489), WeightedEdgeEnd(6, -890238)},
  {
    WeightedEdgeEnd(2, 1238991), WeightedEdgeEnd(1, 1389),
    WeightedEdgeEnd(6, -2319)
  },
  {WeightedEdgeEnd(4, -123), WeightedEdgeEnd(5, 0)}
})));
