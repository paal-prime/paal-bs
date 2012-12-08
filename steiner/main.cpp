#include <iostream>
#include "SteinerForest.h"
#include "../graph/AdjacencyMatrix.h"
#include "../graph/AdjacencyLists.h"
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
    const int vertexCount = 6;
    AdjacencyLists<Graph::undirected, int> graph(vertexCount);


    const int edgeCount = 9;
    Edge edges[edgeCount] = {{0, 1, 16},
                             {0, 2, 6},
                             {1, 2, 12},
                             {2, 3, 9},
                             {3, 4, 6},
                             {0, 4, 20},
                             {4, 5, 19},
                             {3, 5, 12}};

    for(int i = 0; i < edgeCount; ++i)
    {
        graph.add_edge(edges[i].u, edges[i].v, edges[i].weight);
    }


    int sets[vertexCount] = {0, 1, -1, -1, 0, 1}; //-1 = none

    std::vector< std::pair< std::pair<size_t, size_t>, int> > steiner_forest_edges;
    SteinerForest<>(graph, sets, std::back_inserter(steiner_forest_edges));

    std::cout << "result edges:" << std::endl;

    for(size_t i = 0; i < steiner_forest_edges.size(); ++i)
    {
        std::cout << steiner_forest_edges[i].first.first << " " \
                  << steiner_forest_edges[i].first.second << std::endl;
    }

    return 0;
}

