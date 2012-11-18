#ifndef _STEINER_FOREST_H
#define _STEINER_FOREST_H

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <queue>
#include <vector>
#include <functional>

//DEBUG
#include <iostream>

//TODO dzielenie krawedzi przez 2, a zaokraglanie

template <typename G, typename S, typename OutputIterator>
void SteinerForest(const G& graph, const S& sets, OutputIterator steiner_forest_edges)
{
    const typename boost::graph_traits<G>::vertices_size_type verticesCount = num_vertices(graph);
    int distances[verticesCount]; //memset
    bool active[verticesCount]; //memset

    for(size_t i = 0; i < verticesCount; ++i)
    {
        distances[i] = 0;
        active[i] = false;
    }

    typedef typename boost::graph_traits<G>::vertex_iterator vertex_iterator_t;
    std::pair<vertex_iterator_t, vertex_iterator_t> vertex_iterator = vertices(graph);

    int maxSetNumber = 0;
    while(vertex_iterator.first != vertex_iterator.second)
    {
        maxSetNumber = std::max(maxSetNumber, get(sets, *vertex_iterator.first));
        vertex_iterator.first++;
    }

    if(maxSetNumber == -1)
    {
        return;
    }

    const int setsCount = maxSetNumber + 1;
    int compoundTerminals[verticesCount][setsCount]; //memset
    for(size_t i = 0; i < verticesCount; ++i)
    {
        for(int j = 0; j < setsCount; ++j)
        {
            compoundTerminals[i][j] = 0;
        }
    }
    int setCardinality[setsCount]; //memset
    for(int i = 0; i < setsCount; ++i)
    {
        setCardinality[i] = 0;
    }

    //dsu
    typedef typename boost::property_map<G, boost::vertex_index_t>::type vertex_dsu_rank_map_t;
    vertex_dsu_rank_map_t v = get(boost::vertex_index, graph);
    int rank[verticesCount]; //memset
    for(size_t i = 0; i < verticesCount; ++i)
    {
        rank[i] = 0;
    }

    typedef boost::iterator_property_map<int *, vertex_dsu_rank_map_t, int, int&> dsu_rank_t;
    dsu_rank_t dsu_rank(rank, v);

    typedef typename boost::property_map<G, boost::vertex_index_t>::type vertex_dsu_parent_map_t;
    vertex_dsu_parent_map_t p = get(boost::vertex_index, graph);
    int parent[verticesCount]; //memset
    for(size_t i = 0; i < verticesCount; ++i)
    {
        parent[i] = 0;
    }

    typedef boost::iterator_property_map<int *, vertex_dsu_parent_map_t, int, int&> dsu_parent_t;
    dsu_parent_t dsu_parent(parent, p);

    boost::disjoint_sets<dsu_rank_t, dsu_parent_t> dsu(dsu_rank, dsu_parent);

    int activeSets = 0;
    typedef typename boost::graph_traits<G>::out_edge_iterator out_edge_iterator;
    typedef typename boost::graph_traits<G>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_traits<G>::edge_descriptor edge_descriptor;
    typedef std::pair<int, edge_descriptor> queue_element_t;
    std::priority_queue< queue_element_t,
                         std::vector< queue_element_t >,
                         std::greater< queue_element_t > > edge_queue; //zmienic z max na min heap

    vertex_iterator = vertices(graph);
    while(vertex_iterator.first != vertex_iterator.second)
    {
        int set_id = get(sets, *vertex_iterator.first);
        if(set_id != -1)
        {
            setCardinality[set_id] += 1;
        }
        vertex_iterator.first++;
    }

    vertex_iterator = vertices(graph);
    while(vertex_iterator.first != vertex_iterator.second)
    {
        dsu.make_set(*vertex_iterator.first);
        int set_id = get(sets, *vertex_iterator.first);
        if(set_id != -1 && setCardinality[set_id] != 1)
        {
            std::cout << "set vertex " << *vertex_iterator.first << std::endl;
            activeSets += 1;
            active[*vertex_iterator.first] = 1;
            distances[*vertex_iterator.first] = 0;
            compoundTerminals[*vertex_iterator.first][get(sets, *vertex_iterator.first)] += 1;

            //iterate over edges
            out_edge_iterator first;
            out_edge_iterator last;
            boost::tie(first, last) = out_edges(*vertex_iterator.first, graph);
            while(first != last)
            {
                vertex_descriptor source = boost::source(*first, graph);
                vertex_descriptor target = boost::target(*first, graph);
                int weight = 0;
                if(active[target])
                {
                    weight = (distances[target] + distances[source])/2 +
                              get(boost::edge_weight, graph, *first);
                }
                else
                {
                    weight = distances[source] + 2*get(boost::edge_weight, graph, *first);
                }

                edge_queue.push(std::make_pair(weight, *first));

                first++;
            }
        }
        vertex_iterator.first++;
    }

    for(int i = 0; i < setsCount; ++i)
    {
        std::cout << "car " << setCardinality[i] << std::endl;
    }

    while(activeSets)
    {
        std::cout << "sets " << activeSets << std::endl;
        int weight = 0;
        edge_descriptor edge;
        assert(!edge_queue.empty());
        boost::tie(weight, edge) = edge_queue.top();
        edge_queue.pop();
        vertex_descriptor source = boost::source(edge, graph);
        vertex_descriptor target = boost::target(edge, graph);

        vertex_descriptor source_parent = dsu.find_set(source);
        vertex_descriptor target_parent = dsu.find_set(target);
        if(source_parent != target_parent)
        {
            std::cout << "s " << source << ", t " << target << std::endl;
            *steiner_forest_edges++ = edge;

            dsu.link(source, target);
            vertex_descriptor new_root = dsu.find_set(source);
            for(int i = 0; i < setsCount; ++i)
            {
                compoundTerminals[new_root][i] = compoundTerminals[source_parent][i] + \
                                                 compoundTerminals[target_parent][i];
            }
            if(active[target])
            {
                activeSets -= 1;

                bool isActive = false;
                for(int i = 0; i < setsCount; ++i)
                {
                    if(compoundTerminals[new_root][i] > 0 &&
                       setCardinality[i] != compoundTerminals[new_root][i])
                    {
                        isActive = true;
                        break;
                    }
                }
                if(!isActive)
                {
                    activeSets -= 1;
                }
            }
            else
            {
                active[target] = 1;
                distances[target] = weight;
                vertex_descriptor parent = source;
                //copy paste - add adjacent edges, modified (parent)
                out_edge_iterator first;
                out_edge_iterator last;
                boost::tie(first, last) = out_edges(target, graph);
                while(first != last)
                {
                    vertex_descriptor source = boost::source(*first, graph);
                    vertex_descriptor target = boost::target(*first, graph);
                    if(source != parent)
                    {
                        int weight = 0;
                        if(active[target])
                        {
                            weight = (distances[target] + distances[source])/2 +
                                      get(boost::edge_weight, graph, *first);
                        }
                        else
                        {
                            weight = distances[source] + 2*get(boost::edge_weight, graph, *first);
                        }

                        edge_queue.push(std::make_pair(weight, *first));
                    }
                    first++;
                }
                //end copy paste
            }
        }
    }

}

#endif /* _STEINER_FOREST_H */
