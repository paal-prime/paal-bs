#ifndef GRAPH_ADJACENCYMATRIX_H_
#define GRAPH_ADJACENCYMATRIX_H_

#include <utility>
#include <memory>
#include "graph/Graph.h"

template <typename D = Graph::directed, typename W = Graph::unweighted>
class AdjacencyMatrix
{
    template <typename V, typename WW>
    class EdgeEnd
    {
      public:
        V target;
        WW weight;
        EdgeEnd(V t, WW w) : target(t), weight(w) {}
    };

    template <typename M>
    class AdjacencyIterator
    {
      public:
        typedef typename M::vertex_t vertex_t;
        AdjacencyIterator() : adjMatrix_(NULL) {}

        AdjacencyIterator(vertex_t a, vertex_t b, const M* adjMatrix) :
          it(a, b), adjMatrix_(adjMatrix)
        {
          if (!adjMatrix_->adjacent(a, b) && b != adjMatrix_->verticesCount())
          {
            next();
          }
        }
        AdjacencyIterator operator++(int)
        {
          AdjacencyIterator tmp = *this;
          next();
          return tmp;
        }

        void next()
        {
          it.second += 1;
          while (it.second < adjMatrix_->verticesCount()
                 && !(adjMatrix_->adjacent(it.first, it.second)))
          {
            it.second += 1;
          }
        }
        bool operator==(const AdjacencyIterator& rhs) const
        {
          return this->it == rhs.it && this->adjMatrix_ == rhs.adjMatrix_;
        }
        bool operator!=(const AdjacencyIterator& rhs) const
        {
          return !(*this == rhs);
        }
        vertex_t operator*()
        {
          return it.second;
        }
      private:
        std::pair<vertex_t, vertex_t> it;
        const M * adjMatrix_;
    };

    template <typename M, typename WW>
    class EdgeIterator
    {
      public:
        typedef typename M::vertex_t vertex_t;
        typedef typename M::edge_weight_t edge_weight_t;
        EdgeIterator() : adjMatrix_(NULL) {}

        EdgeIterator(vertex_t a, vertex_t b, const M* adjMatrix) :
          it(a, b), adjMatrix_(adjMatrix)
        {
          if (!adjMatrix_->adjacent(a, b) && b != adjMatrix_->verticesCount())
          {
            next();
          }
        }
        EdgeIterator operator++(int)
        {
          EdgeIterator tmp = *this;
          next();
          return tmp;
        }

        void next()
        {
          it.second += 1;
          while (it.second < adjMatrix_->verticesCount()
                 && !(adjMatrix_->adjacent(it.first, it.second)))
          {
            it.second += 1;
          }
        }
        bool operator==(const EdgeIterator& rhs)
        {
          return this->it == rhs.it && this->adjMatrix_ == rhs.adjMatrix_;
        }
        bool operator!=(const EdgeIterator& rhs)
        {
          return !(*this == rhs);
        }


        const EdgeEnd<vertex_t, WW> operator*()
        {
          return EdgeEnd<vertex_t, WW>(it.second,
                 adjMatrix_->edge(it.first, it.second).second);
        }
      private:
        std::pair<vertex_t, vertex_t> it;
        const M * adjMatrix_;
    };

    template <typename M>
    class EdgeIterator<M, Graph::unweighted>;

    template <typename WW, typename V>
    class AdjacencyMatrixFields
    {
      public:
        explicit AdjacencyMatrixFields(V vertices) : vertices_(vertices)
        {
          weights.reset(new WW[vertices_ * vertices_]());
          adjs.reset(new bool[vertices_ * vertices_]());
        }

        W getWeight(V u, V v) const
        {
          return weights[u * vertices_ + v];
        }

        bool getAdj(V u, V v) const
        {
          return adjs[u * vertices_ + v];
        }

        void addEdge(V u, V v, WW w)
        {
          adjs[u * vertices_ + v] = true;
          weights[u * vertices_ + v] = w;
        }

        const V vertices_;
        std::unique_ptr<WW[]> weights;
        std::unique_ptr<bool[]> adjs;
    };

    template <typename V>
    class AdjacencyMatrixFields<Graph::unweighted, V>
    {
      public:
        explicit AdjacencyMatrixFields(V vertices) : vertices_(vertices)
        {
          adjs.reset(new bool[vertices_ * vertices_]());
        }

        bool getAdj(V u, V v) const
        {
          return adjs[u * vertices_ + v];
        }

        void addEdge(V u, V v)
        {
          adjs[u * vertices_ + v] = true;
        }

        const V vertices_;
        std::unique_ptr<bool[]> adjs;
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
    typedef AdjacencyIterator<AdjacencyMatrix<D, W> > adjacency_iterator_t;
    typedef EdgeIterator<AdjacencyMatrix<D, W>, W> edge_iterator_t;

    typedef WeightedEdge<vertex_t, edge_weight_t> weighted_edge_t;
    typedef Edge<vertex_t> edge_t;

    explicit AdjacencyMatrix(size_t size) : fields(size) {}

    std::pair<adjacency_iterator_t, adjacency_iterator_t>
    adjacent_vertices(const vertex_t& v) const
    {
      return std::pair<adjacency_iterator_t, adjacency_iterator_t>(
          adjacency_iterator_t(v, 0, this),
          adjacency_iterator_t(v, verticesCount(), this));
    }

    std::pair<edge_iterator_t, edge_iterator_t> out_edges(const vertex_t& v)
    const
    {
      return std::pair<edge_iterator_t, edge_iterator_t>(
          edge_iterator_t(v, 0, this),
          edge_iterator_t(v, verticesCount(), this));
    }

    bool adjacent(vertex_t u, vertex_t v) const
    {
      return fields.getAdj(u, v);
    }

    std::pair<bool, edge_weight_t> edge(vertex_t u, vertex_t v) const
    {
      return std::pair<bool, edge_weight_t>(fields.getAdj(u, v),
             fields.getWeight(u, v));
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
    }
  private:
    AdjacencyMatrixFields<W, vertex_t> fields;
};

#endif  // GRAPH_ADJACENCYMATRIX_H_
