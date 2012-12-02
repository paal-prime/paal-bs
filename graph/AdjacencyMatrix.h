#ifndef _ADJACENCY_MATRIX_H
#define _ADJACENCY_MATRIX_H

#include <utility>
#include <memory>

namespace Graph
{
    struct directed {};
    struct undirected {};
    struct unweighted {};
}

namespace detail{
        template <typename M>
        class AdjacencyIterator
        {
            public:
                typedef typename M::vertex_t vertex_t;
                AdjacencyIterator(vertex_t a, vertex_t b, M* adjMatrix) :
                    it(a, b), adjMatrix(adjMatrix){}
                AdjacencyIterator& operator++()
                {
                    it.second += 1;
                    while(it.second < adjMatrix->verticesCount()
                          && adjMatrix.edge(it.first, it.second).second);
                    return *this;
                }
                bool operator==(const AdjacencyIterator& rhs)
                {
                    return this->it == rhs.it;
                }
                bool operator!=(const AdjacencyIterator& rhs)
                {
                    return !(*this == rhs);
                }
                vertex_t operator*()
                {
                    return it.second;
                }
            private:
                std::pair<int, int> it;
                M* adjMatrix;
        };

        template <typename M>
        class EdgeIterator
        {
            public:
                typedef typename M::vertex_t vertex_t;
                typedef typename M::edge_t edge_t;
                EdgeIterator(vertex_t a, vertex_t b, M* adjMatrix) :
                    it(a, b), adjMatrix(adjMatrix){}
                EdgeIterator& operator++()
                {
                    it.second += 1;
                    while(it.second < adjMatrix->verticesCount()
                          && adjMatrix.edge(it.first, it.second).second);
                    return *this;
                }
                bool operator==(const EdgeIterator& rhs)
                {
                    return this->it == rhs.it;
                }
                bool operator!=(const EdgeIterator& rhs)
                {
                    return !(*this == rhs);
                }
                edge_t operator*()
                {
                    return it;
                }
            private:
                std::pair<int, int> it;
                M* adjMatrix;
        };

        template <typename W>
        class AdjacencyMatrixFields
        {
            public:
                AdjacencyMatrixFields(int vertices) : vertices(vertices)
                {
                    weights.reset(new W[vertices * vertices]);
                    adjs.reset(new bool[vertices * vertices]);
                }

                W getWeight(int u, int v)
                {
                    return weights[u * vertices + v];
                }
                void setWeight(int u, int v, W w)
                {
                    weights[u * vertices + v] = w;
                }

                bool getAdj(int u, int v)
                {
                    return adjs[u * vertices + v];
                }

                void setAdj(int u, int v, bool b)
                {
                    adjs[u * vertices + v] = b;
                }

                const int vertices;
                std::unique_ptr<W[]> weights;
                std::unique_ptr<bool[]> adjs;
        };

        template <>
        class AdjacencyMatrixFields<Graph::unweighted>
        {
            public:
                AdjacencyMatrixFields(int vertices) : vertices(vertices)
                {
                    adjs.reset(new bool[vertices * vertices]);
                }

                bool getAdj(int u, int v)
                {
                    return adjs[u * vertices + v];
                }

                void setAdj(int u, int v, bool b)
                {
                    adjs[u * vertices + v] = b;
                }

                const int vertices;
                std::unique_ptr<bool[]> adjs;
        };
}


template <typename D, typename W>
class AdjacencyMatrix
{
    public:
        typedef int vertex_t;
        typedef detail::AdjacencyIterator<AdjacencyMatrix<D, W> > adjacency_iterator_t;
        typedef std::pair<int, int> edge_t;
        typedef detail::EdgeIterator<AdjacencyMatrix<D, W> > edge_iterator_t;
        typedef W edge_weight_t;

        AdjacencyMatrix(int size);

        vertex_t source(const edge_t &e);
        vertex_t target(const edge_t &e);

        std::pair<adjacency_iterator_t, adjacency_iterator_t> adjacent_vertices(const vertex_t& v);
        std::pair<edge_iterator_t, edge_iterator_t> out_edges(const vertex_t& v);

        std::pair<edge_t, bool> edge(vertex_t u, vertex_t v);

        edge_weight_t weight(edge_t edge);
        edge_weight_t weight(edge_iterator_t e_it);

        void add_edge(vertex_t u, vertex_t v); //TODO change AdjMatFields<W>
                                               //to <D, W> and specialize it
        void add_edge(vertex_t u, vertex_t v, edge_weight_t w); //TODO

        int verticesCount();
    private:
        detail::AdjacencyMatrixFields<W> fields;
};

template <typename D, typename W>
AdjacencyMatrix<D, W>::AdjacencyMatrix(int size) : fields(size){}

template <typename D, typename W>
std::pair<typename AdjacencyMatrix<D, W>::edge_t, bool> AdjacencyMatrix<D, W>::edge(
            typename AdjacencyMatrix<D, W>::vertex_t u, typename AdjacencyMatrix<D, W>::vertex_t v)
{
    typedef typename AdjacencyMatrix<D, W>::edge_t edge_t;
    typedef typename AdjacencyMatrix<D, W>::vertex_t vertex_t;
    return std::pair<edge_t, bool>(edge_t(u, v), fields.getAdj(u, v));
}

template <typename D, typename W>
typename AdjacencyMatrix<D, W>::edge_weight_t AdjacencyMatrix<D, W>::weight(
        typename AdjacencyMatrix<D, W>::edge_t edge)
{
    return fields.getWeight(edge.first, edge.second);
}


template <typename D, typename W>
typename AdjacencyMatrix<D, W>::edge_weight_t AdjacencyMatrix<D, W>::weight(
        typename AdjacencyMatrix<D, W>::edge_iterator_t e_it)
{
    return fields.getWeight((*e_it).first, (*e_it).second);
}

template <typename D, typename W>
std::pair<
    typename AdjacencyMatrix<D, W>::adjacency_iterator_t,
    typename AdjacencyMatrix<D, W>::adjacency_iterator_t
         >
AdjacencyMatrix<D, W>::adjacent_vertices(const typename AdjacencyMatrix<D, W>::vertex_t& v)
{
    typedef typename AdjacencyMatrix<D, W>::adjacency_iterator_t adjacency_iterator_t;
    return std::pair<adjacency_iterator_t, adjacency_iterator_t>(
                         adjacency_iterator_t(v, 0),
                         adjacency_iterator_t(v, verticesCount()),
                         this);
}


template <typename D, typename W>
std::pair<
    typename AdjacencyMatrix<D, W>::edge_iterator_t,
    typename AdjacencyMatrix<D, W>::edge_iterator_t
         >
AdjacencyMatrix<D, W>::out_edges(const typename AdjacencyMatrix<D, W>::vertex_t& v)
{
    typedef typename AdjacencyMatrix<D, W>::edge_iterator_t edge_iterator_t;
    return std::pair<edge_iterator_t, edge_iterator_t>(
                         edge_iterator_t(v, 0),
                         edge_iterator_t(v, verticesCount()),
                         this);
}


template <typename D, typename W>
typename AdjacencyMatrix<D, W>::vertex_t AdjacencyMatrix<D, W>::source(const edge_t &e)
{
    return e.first;
}

template <typename D, typename W>
typename AdjacencyMatrix<D, W>::vertex_t AdjacencyMatrix<D, W>::target(const edge_t &e)
{
    return e.second;
}

template <typename D, typename W>
int AdjacencyMatrix<D, W>::verticesCount()
{
    return fields.vertices;
}


#endif /* _ADJACENCY_MATRIX_H */
