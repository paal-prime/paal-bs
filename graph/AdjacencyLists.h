#ifndef _ADJACENCY_LISTS_H
#define _ADJACENCY_LISTS_H

#include "Graph.h"
#include <list>

namespace details
{
    template <typename T>
    class adj_list_AdjacencyIterator
    {
        public:
        adj_list_AdjacencyIterator() {};
        adj_list_AdjacencyIterator(T it) : it(it) {}
        adj_list_AdjacencyIterator operator++(int)
        {
            adj_list_AdjacencyIterator tmp = *this;
            it++;
            return tmp;
        }

        template <typename V>
        V operator*()
        {
            return it->first;
        }
        private:
        T it;
    };

    template <typename W, typename V>
    struct adj_list_adjacency_iterator
    {
        typedef adj_list_AdjacencyIterator<typename std::list<V>::iterator> type;
    };

    template <typename V>
    struct adj_list_adjacency_iterator<Graph::unweighted, V>
    {
        typedef typename std::list<V>::iterator type;
    };
}


template <typename D = Graph::directed, typename W = Graph::unweighted>
class AdjacencyLists
{
    template <typename WW, typename V>
    class AdjacencyListsFields
    {
        public:
        AdjacencyListsFields(V vertices) : vertices(vertices)
        {
            adj.reset(new std::list< std::pair<V, WW> >[vertices]);
        }

        bool getAdj(V u, V v) const
        {
            typename std::list<V>::iterator it = adj[u].begin();
            while(it != adj[u].end())
            {
                if(it->first == v)
                {
                    return true;
                }
                it++;
            }

            return false;
        }

        std::pair<bool, WW> edge(V u, V v)
        {
            typename std::list<V>::iterator it = adj[u].begin();
            while(it != adj[u].end())
            {
                if(it->first == v)
                {
                    return *it;
                }
                it++;
            }

            return std::make_pair(false, WW());
        }

        void addEdge(V u, V v, WW w)
        {
            adj[u].push_back(std::make_pair(v, w));
        }

        V vertices;
        std::unique_ptr<std::list< std::pair<V, WW> >[]> adj;
    };

    template <typename V>
    class AdjacencyListsFields<Graph::unweighted, V>
    {
        public:
        AdjacencyListsFields(V vertices) : vertices(vertices)
        {
            adj.reset(new std::list<V>[vertices]);
        }

        bool getAdj(V u, V v) const
        {
            typename std::list<V>::iterator it = adj[u].begin();
            while(it != adj[u].end())
            {
                if(*it == v)
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

        V vertices;
        std::unique_ptr<std::list<V>[]> adj;
    };

    public:
    typedef size_t vertex_t;
    typedef W edge_weight_t;
    typedef typename details::adj_list_adjacency_iterator<W, vertex_t>::type adjacency_iterator_t;
    typedef typename std::list< std::pair<vertex_t, edge_weight_t> >::iterator edge_iterator_t;

    AdjacencyLists(size_t size) : fields(size) {}

    std::pair<adjacency_iterator_t, adjacency_iterator_t> adjacent_vertices(const vertex_t& v)
    {
        return make_pair(adjacency_iterator_t(fields.adj[v].begin()),
                         adjacency_iterator_t(fields.adj[v].end()));
    }

    std::pair<edge_iterator_t, edge_iterator_t> out_edges(const vertex_t& v) const
    {
        return std::make_pair(fields.adj[v].begin(), fields.adj[v].end());
    }

    bool adjacent(vertex_t u, vertex_t v) const
    {
        return fields.getAdj(u, v);
    }

    std::pair<bool, edge_weight_t> edge(vertex_t u, vertex_t v) const
    {
        fields.edge(u, v);
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
    };

    private:
    AdjacencyListsFields<W, vertex_t> fields;
};

#endif /* _ADJACENCY_LISTS_H */
