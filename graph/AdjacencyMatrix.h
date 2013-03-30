#ifndef GRAPH_ADJACENCYMATRIX_H_
#define GRAPH_ADJACENCYMATRIX_H_

#include <utility>
#include <memory>
#include "graph/Graph.h"

/**
 * Representation of graph using adjacency matrix.
 * @tparam D determines whether graph is directed or not.
 * Possible values are graph::directed and graph::undirected.
 * @tparam W codomain of weight/cost function,
 * use graph::unweighted for unweighted graphs.
 **/
template <typename D = graph::directed, typename W = graph::unweighted>
class AdjacencyMatrix
{
    template <typename M>
    class AdjacencyIterator
    {
      public:
        typedef typename M::vertex_t vertex_t;

        AdjacencyIterator() : adj_matrix_(NULL) {}

        AdjacencyIterator(const vertex_t& source, const vertex_t& target,
            const M* adj_matrix)
          : it_(source, target), adj_matrix_(adj_matrix)
        {
          if (!adj_matrix_->adjacent(source, target)
              && target != adj_matrix_->get_vertices_count())
          {
            next();
          }
        }

        AdjacencyIterator& operator++()
        {
          next();
          return *this;
        }

        AdjacencyIterator operator++(int)
        {
          AdjacencyIterator prev = *this;
          next();
          return prev;
        }

        void next()
        {
          it_.second += 1;
          while (it_.second < adj_matrix_->get_vertices_count()
                 && !(adj_matrix_->adjacent(it_.first, it_.second)))
          {
            it_.second += 1;
          }
        }

        bool operator==(const AdjacencyIterator& rhs) const
        {
          return this->it_ == rhs.it_ && this->adj_matrix_ == rhs.adj_matrix_;
        }

        bool operator!=(const AdjacencyIterator& rhs) const
        {
          return !operator==(rhs);
        }

        const vertex_t& operator*() const
        {
          return it_.second;
        }

      private:
        std::pair<vertex_t, vertex_t> it_;
        const M * adj_matrix_;
    };

    template <typename M, typename WW>
    class EdgeIterator
    {
      public:
        typedef typename M::vertex_t vertex_t;
        typedef typename M::edge_weight_t edge_weight_t;

        EdgeIterator() : adj_matrix_(NULL) {}

        EdgeIterator(const vertex_t& source, const vertex_t& target,
            const M* adj_matrix) :
          it_(source, target), adj_matrix_(adj_matrix)
        {
          if (!adj_matrix_->adjacent(source, target)
              && target != adj_matrix_->get_vertices_count())
          {
            next();
          }
        }

        EdgeIterator& operator++()
        {
          next();
          return *this;
        }

        EdgeIterator operator++(int)
        {
          EdgeIterator prev = *this;
          next();
          return prev;
        }

        void next()
        {
          it_.second += 1;
          while (it_.second < adj_matrix_->get_vertices_count()
                 && !(adj_matrix_->adjacent(it_.first, it_.second)))
          {
            it_.second += 1;
          }
        }

        bool operator==(const EdgeIterator& rhs) const
        {
          return this->it_ == rhs.it_ && this->adj_matrix_ == rhs.adj_matrix_;
        }

        bool operator!=(const EdgeIterator& rhs) const
        {
          return !operator==(rhs);
        }

        const graph::EdgeEnd<vertex_t, WW> operator*() const
        {
          return graph::EdgeEnd<vertex_t, WW>(it_.second,
                 adj_matrix_->edge(it_.first, it_.second).second);
        }

      private:
        std::pair<vertex_t, vertex_t> it_;
        const M * adj_matrix_;
    };

    template <typename M>
    class EdgeIterator<M, graph::unweighted>;

    template <typename WW, typename V>
    class AdjacencyMatrixFields
    {
      public:
        explicit AdjacencyMatrixFields(const V& vertices) : vertices_(vertices)
        {
          weights_.reset(new WW[vertices_ * vertices_]());
          adjs_.reset(new bool[vertices_ * vertices_]());
        }

        W get_weight(const V& u, const V& v) const
        {
          return weights_[u * vertices_ + v];
        }

        bool get_adj(const V& u, const V& v) const
        {
          return adjs_[u * vertices_ + v];
        }

        void add_edge(const V& u, const V& v, const WW& w)
        {
          adjs_[u * vertices_ + v] = true;
          weights_[u * vertices_ + v] = w;
        }

        const V vertices_;
        std::unique_ptr<WW[]> weights_;
        std::unique_ptr<bool[]> adjs_;
    };

    template <typename V>
    class AdjacencyMatrixFields<graph::unweighted, V>
    {
      public:
        explicit AdjacencyMatrixFields(const V& vertices) : vertices_(vertices)
        {
          adjs_.reset(new bool[vertices_ * vertices_]());
        }

        bool get_adj(const V& u, const V& v) const
        {
          return adjs_[u * vertices_ + v];
        }

        void add_edge(const V& u, const V& v)
        {
          adjs_[u * vertices_ + v] = true;
        }

        const V vertices_;
        std::unique_ptr<bool[]> adjs_;
    };

  public:
    typedef size_t vertex_t;
    typedef W edge_weight_t;

    typedef AdjacencyIterator<AdjacencyMatrix<D, W> > adjacency_iterator_t;
    typedef EdgeIterator<AdjacencyMatrix<D, W>, W> edge_iterator_t;

    typedef graph::WeightedEdge<vertex_t, edge_weight_t> weighted_edge_t;
    typedef graph::Edge<vertex_t> edge_t;

    explicit AdjacencyMatrix(const size_t& vertices_count)
      : fields_(vertices_count) {}

    /**
     * @returns pair of iterators to list of vertices adjacent to `v`,
     * first points to beginning of the list and second to end of the list.
     **/
    std::pair<adjacency_iterator_t, adjacency_iterator_t>
    adjacent_vertices(const vertex_t& v) const
    {
      return std::pair<adjacency_iterator_t, adjacency_iterator_t>(
          adjacency_iterator_t(v, 0, this),
          adjacency_iterator_t(v, get_vertices_count(), this));
    }

    /**
     * @returns pair of iterators to list of vertex's `v` out edges,
     * first points to beginning of the list and second to end of the list.
     **/
    std::pair<edge_iterator_t, edge_iterator_t> out_edges(const vertex_t& v)
    const
    {
      return std::pair<edge_iterator_t, edge_iterator_t>(
          edge_iterator_t(v, 0, this),
          edge_iterator_t(v, get_vertices_count(), this));
    }

    /**
     * @returns true if there exists edge (u, v) in graph, false otherwise.
     **/
    bool adjacent(const vertex_t& u, const vertex_t& v) const
    {
      return fields_.get_adj(u, v);
    }

    /**
     * @returns pair of values. First value is true if there exists
     * edge (u, v) in graph and false otherwise. If first value is true
     * then second value is weight of the edge, otherwise it's undefined.
     **/
    std::pair<bool, edge_weight_t> edge(const vertex_t& u, const vertex_t& v)
    const
    {
      return std::pair<bool, edge_weight_t>(fields_.get_adj(u, v),
             fields_.get_weight(u, v));
    }

   /**
    * @brief adds edge (u, v). If graph is undirected it also adds edge (v, u).
    **/
    void add_edge(const vertex_t& u, const vertex_t& v)
    {
      fields_.add_edge(u, v);
      if (!D::is_directed)
      {
        fields_.add_edge(v, u);
      }
    }

    /**
     * @brief adds edge (u, v) of weight w. If graph is undirected it also adds edge (v, u)
     * of the same weight.
     **/
    void add_edge(const vertex_t& u, const vertex_t& v, const edge_weight_t& w)
    {
      fields_.add_edge(u, v, w);
      if (!D::is_directed)
      {
        fields_.add_edge(v, u, w);
      }
    }

    /**
     * @returns number of vertices in graph.
     **/
    size_t get_vertices_count() const
    {
      return fields_.vertices_;
    }

  private:
    AdjacencyMatrixFields<W, vertex_t> fields_;
};

#endif  // GRAPH_ADJACENCYMATRIX_H_
