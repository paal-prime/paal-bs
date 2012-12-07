#ifndef _ADJACENCY_MATRIX_H
#define _ADJACENCY_MATRIX_H

#include <utility>
#include <memory>
#include "Graph.h"

template <typename D = Graph::directed, typename W = Graph::unweighted>
class AdjacencyMatrix
{
    template <typename M>
    class AdjacencyIterator
    {
        public:
            typedef typename M::vertex_t vertex_t;
            AdjacencyIterator(vertex_t a, vertex_t b, M* adjMatrix) :
                it(a, b), adjMatrix(adjMatrix)
            {
                if(!adjMatrix->adjacent(a, b) && b != adjMatrix->verticesCount())
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
                while(it.second < adjMatrix->verticesCount()
                      && !(adjMatrix->adjacent(it.first, it.second)))
                {
                    it.second += 1;
                }
            }
            bool operator==(const AdjacencyIterator& rhs) const
            {
                return this->it == rhs.it && this->adjMatrix == rhs.adjMatrix;
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
            const M * const adjMatrix;
    };

    template <typename M, typename WW>
    class EdgeIterator
    {
        public:
            typedef typename M::vertex_t vertex_t;
            typedef typename M::edge_weight_t edge_weight_t;
            EdgeIterator(vertex_t a, vertex_t b, M* adjMatrix) :
                it(a, b), adjMatrix(adjMatrix)
            {
                if(!adjMatrix->adjacent(a, b) && b != adjMatrix->verticesCount())
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
                while(it.second < adjMatrix->verticesCount()
                      && !(adjMatrix->adjacent(it.first, it.second)))
                {
                    it.second += 1;
                }
            }
            bool operator==(const EdgeIterator& rhs)
            {
                return this->it == rhs.it && this->adjMatrix == rhs.adjMatrix;
            }
            bool operator!=(const EdgeIterator& rhs)
            {
                return !(*this == rhs);
            }
            std::pair<vertex_t, edge_weight_t> operator*()
            {
                return std::pair<vertex_t, edge_weight_t>(it.second, adjMatrix->edge(it.first, it.second).second);
            }
        private:
            std::pair<vertex_t, vertex_t> it;
            const M * const adjMatrix;
    };

    template <typename M>
    class EdgeIterator<M, Graph::unweighted>;

    template <typename WW, typename V>
    class AdjacencyMatrixFields
    {
        public:
            AdjacencyMatrixFields(V vertices) : vertices(vertices)
            {
                weights.reset(new WW[vertices * vertices]);
                adjs.reset(new bool[vertices * vertices]);
            }

            W getWeight(V u, V v) const
            {
                return weights[u * vertices + v];
            }

            bool getAdj(V u, V v) const
            {
                return adjs[u * vertices + v];
            }

            void addEdge(V u, V v, WW w)
            {
                adjs[u * vertices + v] = true;
                weights[u * vertices + v] = w;
            }

            const V vertices;
            std::unique_ptr<WW[]> weights;
            std::unique_ptr<bool[]> adjs;
    };

    template <typename V>
    class AdjacencyMatrixFields<Graph::unweighted, V>
    {
        public:
            AdjacencyMatrixFields(V vertices) : vertices(vertices)
            {
                adjs.reset(new bool[vertices * vertices]);
            }

            bool getAdj(V u, V v) const
            {
                return adjs[u * vertices + v];
            }

            void addEdge(V u, V v)
            {
                adjs[u * vertices + v] = true;
            }

            const V vertices;
            std::unique_ptr<bool[]> adjs;
    };

    public:
        typedef size_t vertex_t;
        typedef W edge_weight_t;
        typedef AdjacencyIterator<AdjacencyMatrix<D, W> > adjacency_iterator_t;
        typedef EdgeIterator<AdjacencyMatrix<D, W>, W> edge_iterator_t;

        AdjacencyMatrix(size_t size) : fields(size) {}

        std::pair<adjacency_iterator_t, adjacency_iterator_t> adjacent_vertices(const vertex_t& v)
        {

            return std::pair<adjacency_iterator_t, adjacency_iterator_t>(
                                 adjacency_iterator_t(v, 0, this),
                                 adjacency_iterator_t(v, verticesCount(), this));

        }
        std::pair<edge_iterator_t, edge_iterator_t> out_edges(const vertex_t& v)
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
            return std::pair<bool, edge_weight_t>(fields.getAdj(u, v), fields.getWeight(u, v));
        }

        void add_edge(vertex_t u, vertex_t v)
        {
            fields.addEdge(u, v);
            if(!D::is_directed)
            {
                fields.addEdge(v, u);
            }
        }

        void add_edge(vertex_t u, vertex_t v, edge_weight_t w)
        {
            fields.addEdge(u, v, w);
            if(!D::is_directed)
            {
                fields.addEdge(v, u, w);
            }
        }

        size_t verticesCount() const
        {
            return fields.vertices;
        }
    private:
        AdjacencyMatrixFields<W, vertex_t> fields;
};

#endif /* _ADJACENCY_MATRIX_H */
