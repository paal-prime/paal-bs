#include <iostream>
#include "SteinerForest.h"
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/graph_utility.hpp>
#include <vector>

struct vertex_set_t {
    typedef boost::vertex_property_tag type;
};

struct Edge
{
    int u;
    int v;
    int weight;
};

int main()
{
    //example graph - Vazirani, page 202
    const int vertexCount = 6;
    typedef boost::adjacency_matrix<boost::undirectedS,
                                    boost::no_property,
                                    boost::property<boost::edge_weight_t, int> > graph_t;
    graph_t graph(vertexCount);

 //   const char * names = "usabvt";

    const int edgeCount = 9;
    Edge edges[edgeCount] = {{0, 1, 16},
                             {0, 2, 6},
                             {1, 2, 12},
                             {1, 2, 6},
                             {2, 3, 9},
                             {3, 4, 6},
                             {0, 4, 20},
                             {4, 5, 19},
                             {3, 5, 12}};

    for(int i = 0; i < edgeCount; ++i)
    {
        boost::add_edge(edges[i].u, edges[i].v, edges[i].weight, graph);
    }

    //vertices sets, 0 - none
    typedef boost::property_map<graph_t, boost::vertex_index_t>::type vertex_set_map_t;
    vertex_set_map_t v = get(boost::vertex_index, graph);
    int sets[vertexCount] = {0, 1, -1, -1, 0, 1}; //-1 = none
    typedef boost::iterator_property_map<int *, vertex_set_map_t, int, int&> sets_t;
    sets_t sets_map(sets, v);
//    boost::print_graph(graph, names);

    typedef typename boost::graph_traits<graph_t>::edge_descriptor edge_t;
    std::vector<edge_t> steiner_forest_edges;
    SteinerForest<graph_t, sets_t>(graph, sets_map, std::back_inserter(steiner_forest_edges));

    for(size_t i = 0; i < steiner_forest_edges.size(); ++i)
    {
        std::cout << steiner_forest_edges[i] << std::endl;
    }

    return 0;
}

