#ifndef GRAPH_ADJACENCYLISTS_H_
#define GRAPH_ADJACENCYLISTS_H_

#include "graph/Graph.h"
#include <list>
#include <utility>



namespace details
{

  template <typename V, typename WW>
  class EdgeEnd
  {
    public:
      V target;
      WW weight;
      EdgeEnd(V t, WW w) : target(t), weight(w) {}
  };


  template <typename T, typename V>
  class adj_list_AdjacencyIterator
  {
    public:
      adj_list_AdjacencyIterator() {}
      explicit adj_list_AdjacencyIterator(T iterator) : iterator_(iterator) {}
      adj_list_AdjacencyIterator operator++(int)
      {
        adj_list_AdjacencyIterator tmp = *this;
        iterator_++;
        return tmp;
      }

      V operator*()
      {
        return iterator_->target;
      }

      bool operator==(const adj_list_AdjacencyIterator& rhs) const
      {
        return this->iterator_ == rhs.iterator_;
      }

      bool operator!=(const adj_list_AdjacencyIterator& rhs) const
      {
        return !(this->operator==(rhs));
      }
    private:
      T iterator_;
  };

  template <typename W, typename V>
  struct adj_list_adjacency_iterator
  {
    typedef adj_list_AdjacencyIterator
    <typename std::list< EdgeEnd<V, W> >::iterator, V> type;
  };

  template <typename V>
  struct adj_list_adjacency_iterator<Graph::unweighted, V>
  {
    typedef typename std::list<V>::iterator type;
  };
}  // namespace details


template <typename D = Graph::directed, typename W = Graph::unweighted>
class AdjacencyLists
{
    template <typename WW, typename V>
    class AdjacencyListsFields
    {
      public:
        explicit AdjacencyListsFields(V vertices) : vertices_(vertices)
        {
          adj.reset(new std::list< details::EdgeEnd<V, WW> >[vertices_]);
        }

        bool getAdj(V u, V v) const
        {
          typename std::list< details::EdgeEnd<V, WW> >::iterator it =
            adj[u].begin();
          while (it != adj[u].end())
          {
            if (it->target == v)
            {
              return true;
            }
            it++;
          }

          return false;
        }

        std::pair<bool, WW> edge(V u, V v) const
        {
          typename std::list< details::EdgeEnd<V, WW> >::iterator it =
            adj[u].begin();
          while (it != adj[u].end())
          {
            if (it->target == v)
            {
              return std::pair<bool, WW>(true, it->weight);
            }
            it++;
          }

          return std::make_pair(false, WW());
        }

        void addEdge(V u, V v, WW w)
        {
          adj[u].push_back(details::EdgeEnd<V, WW>(v, w));
        }

        V vertices_;
        std::unique_ptr<std::list< details::EdgeEnd<V, WW> >[]> adj;
    };

    template <typename V>
    class AdjacencyListsFields<Graph::unweighted, V>
    {
      public:
        explicit AdjacencyListsFields(V vertices) : vertices_(vertices)
        {
          adj.reset(new std::list<V>[vertices_]);
        }

        bool getAdj(V u, V v) const
        {
          typename std::list<V>::iterator it = adj[u].begin();
          while (it != adj[u].end())
          {
            if (*it == v)
            {
              return true;
            }
            it++;
          }

          return false;
        }

        void addEdge(V u, V v)
        {
          adj[u].push_back(v);
        }

        V vertices_;
        std::unique_ptr<std::list<V>[]> adj;
    };


    template <typename V>
    class Edge
    {
      public:
        V source;
        V target;
        Edge(V s, V t) : source(s), target(t) {}
        Edge() : source(0), target(0) {}

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
    };

    template <typename V, typename WW>
    class WeightedEdge
    {
      public:
        V source;
        V target;
        WW weight;
        WeightedEdge(V s, V t, WW w) : source(s), target(t), weight(w) {}
    };

  public:
    typedef size_t vertex_t;
    typedef W edge_weight_t;
    typedef typename details::adj_list_adjacency_iterator<W, vertex_t>::type
    adjacency_iterator_t;
    typedef typename std::list< details::EdgeEnd<vertex_t, W> >::iterator
    edge_iterator_t;

    typedef WeightedEdge<vertex_t, edge_weight_t> weighted_edge_t;
    typedef Edge<vertex_t> edge_t;

    explicit AdjacencyLists(size_t size) : fields(size) {}

    std::pair<adjacency_iterator_t, adjacency_iterator_t>
    adjacent_vertices(const vertex_t& v) const
    {
      return make_pair(adjacency_iterator_t(fields.adj[v].begin()),
             adjacency_iterator_t(fields.adj[v].end()));
    }

    std::pair<edge_iterator_t, edge_iterator_t> out_edges(const vertex_t& v)
    const
    {
      return std::make_pair(fields.adj[v].begin(), fields.adj[v].end());
    }

    bool adjacent(vertex_t u, vertex_t v) const
    {
      return fields.getAdj(u, v);
    }

    std::pair<bool, edge_weight_t> edge(vertex_t u, vertex_t v) const
    {
      return fields.edge(u, v);
    }

    void add_edge(vertex_t u, vertex_t v)
    {
      fields.addEdge(u, v);
      if (!D::is_directed)
      {
        fields.addEdge(v, u);
      }
    }

    void add_edge(vertex_t u, vertex_t v, edge_weight_t w)
    {
      fields.addEdge(u, v, w);
      if (!D::is_directed)
      {
        fields.addEdge(v, u, w);
      }
    }

    size_t verticesCount() const
    {
      return fields.vertices_;
    };

  private:
    AdjacencyListsFields<W, vertex_t> fields;
};

#endif  // GRAPH_ADJACENCYLISTS_H_
