#ifndef GRAPH_ADJACENCYLISTS_H_
#define GRAPH_ADJACENCYLISTS_H_

#include "graph/Graph.h"
#include <list>
#include <utility>

/**
 * Representation of graph using adjacency lists.
 * @tparam D determines whether graph is directed or not.
 * Possible values are Graph::directed and Graph::undirected.
 * @tparam W codomain of weight/cost function,
 * use Graph::unweighted for unweighted graphs.
 **/
template <typename D = Graph::directed, typename W = Graph::unweighted>
class AdjacencyLists
{
    template <typename T, typename V>
    class AdjacencyIterator
    {
      public:
        AdjacencyIterator() {}
        explicit AdjacencyIterator(const T& iterator) : iterator_(iterator) {}

        AdjacencyIterator& operator++()
        {
          ++iterator_;
          return *this;
        }

        AdjacencyIterator operator++(int)
        {
          AdjacencyIterator prev = *this;
          iterator_++;
          return prev;
        }

        const V& operator*() const
        {
          return iterator_->target;
        }

        bool operator==(const AdjacencyIterator& rhs) const
        {
          return this->iterator_ == rhs.iterator_;
        }

        bool operator!=(const AdjacencyIterator& rhs) const
        {
          return !operator==(rhs);
        }

      private:
        T iterator_;
    };

    template <typename WW, typename V>
    class AdjacencyIteratorTypeWrapper
    {
      public:
        typedef AdjacencyIterator
        <typename std::list< Graph::EdgeEnd<V, WW> >::iterator, V> type;
    };

    template <typename V>
    class AdjacencyIteratorTypeWrapper<Graph::unweighted, V>
    {
      public:
        typedef typename std::list<V>::iterator type;
    };

    template <typename WW, typename V>
    class AdjacencyListsFields
    {
      public:
        explicit AdjacencyListsFields(const V& vertices) : vertices_(vertices)
        {
          adj_.reset(new std::list< Graph::EdgeEnd<V, WW> >[vertices_]);
        }

        bool getAdj(const V& u, const V& v) const
        {
          typename std::list< Graph::EdgeEnd<V, WW> >::iterator it =
            adj_[u].begin();
          while (it != adj_[u].end())
          {
            if (it->target == v)
            {
              return true;
            }
            it++;
          }

          return false;
        }

        std::pair<bool, WW> edge(const V& u, const V& v) const
        {
          typename std::list< Graph::EdgeEnd<V, WW> >::iterator it =
            adj_[u].begin();
          while (it != adj_[u].end())
          {
            if (it->target == v)
            {
              return std::pair<bool, WW>(true, it->weight);
            }
            it++;
          }

          return std::make_pair(false, WW());
        }

        void addEdge(const V& u, const V& v, const WW& w)
        {
          adj_[u].push_back(Graph::EdgeEnd<V, WW>(v, w));
        }

        V vertices_;
        std::unique_ptr<std::list< Graph::EdgeEnd<V, WW> >[]> adj_;
    };

    template <typename V>
    class AdjacencyListsFields<Graph::unweighted, V>
    {
      public:
        explicit AdjacencyListsFields(const V& vertices) : vertices_(vertices)
        {
          adj_.reset(new std::list<V>[vertices_]);
        }

        bool getAdj(const V& u, const V& v) const
        {
          typename std::list<V>::iterator it = adj_[u].begin();
          while (it != adj_[u].end())
          {
            if (*it == v)
            {
              return true;
            }
            it++;
          }

          return false;
        }

        void addEdge(const V& u, const V& v)
        {
          adj_[u].push_back(v);
        }

        V vertices_;
        std::unique_ptr<std::list<V>[]> adj_;
    };

  public:
    typedef size_t vertex_t;
    typedef W edge_weight_t;

    typedef typename AdjacencyIteratorTypeWrapper<W, vertex_t>::type
    adjacency_iterator_t;
    typedef typename std::list< Graph::EdgeEnd<vertex_t, W> >::iterator
    edge_iterator_t;

    typedef Graph::WeightedEdge<vertex_t, edge_weight_t> weighted_edge_t;
    typedef Graph::Edge<vertex_t> edge_t;

    explicit AdjacencyLists(const size_t& vertices_count)
      : fields_(vertices_count) {}

    /**
     * @returns pair of iterators to list of vertices adjacent to `v`,
     * first points to beginning of the list and second to end of the list.
     **/
    std::pair<adjacency_iterator_t, adjacency_iterator_t>
    adjacent_vertices(const vertex_t& v) const
    {
      return make_pair(adjacency_iterator_t(fields_.adj_[v].begin()),
             adjacency_iterator_t(fields_.adj_[v].end()));
    }

    /**
     * @returns pair of iterators to list of vertex's `v` out edges,
     * first points to beginning of the list and second to end of the list.
     **/
    std::pair<edge_iterator_t, edge_iterator_t> out_edges(const vertex_t& v)
    const
    {
      return std::make_pair(fields_.adj_[v].begin(), fields_.adj_[v].end());
    }

    /**
     * @returns true if there exists edge (u, v) in graph, false otherwise.
     **/
    bool adjacent(const vertex_t& u, const vertex_t& v) const
    {
      return fields_.getAdj(u, v);
    }

    /**
     * @returns pair of values. First value is true if there exists
     * edge (u, v) in graph and false otherwise. If first value is true
     * then second value is weight of the edge, otherwise it's undefined.
     **/
    std::pair<bool, edge_weight_t> edge(const vertex_t& u, const vertex_t& v)
    const
    {
      return fields_.edge(u, v);
    }

   /**
    * @brief adds edge (u, v). If graph is undirected it also adds edge (v, u).
    **/
    void add_edge(const vertex_t& u, const vertex_t& v)
    {
      fields_.addEdge(u, v);
      if (!D::is_directed)
      {
        fields_.addEdge(v, u);
      }
    }

    /**
     * @brief adds edge (u, v) of weight w. If graph is undirected it also adds edge (v, u)
     * of the same weight.
     **/
    void add_edge(const vertex_t& u, const vertex_t& v, const edge_weight_t& w)
    {
      fields_.addEdge(u, v, w);
      if (!D::is_directed)
      {
        fields_.addEdge(v, u, w);
      }
    }

    /**
     * @returns number of vertices in graph.
     **/
    size_t verticesCount() const
    {
      return fields_.vertices_;
    };

  private:
    AdjacencyListsFields<W, vertex_t> fields_;
};

#endif  // GRAPH_ADJACENCYLISTS_H_
