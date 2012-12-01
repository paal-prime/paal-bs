#ifndef _STEINER_FOREST_H
#define _STEINER_FOREST_H

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <queue>
#include <vector>
#include <list>
#include <stack>
#include <functional>
#include <cmath>

//DEBUG
#include <iostream>

void getParents(const std::vector< std::list<int> >& adj_lists, int parent[])
{
    const int verticesCount = adj_lists.size();
    bool visited[verticesCount];
    memset(visited, false, sizeof(bool) * verticesCount);
    typedef std::list<int>::const_iterator list_it_t;
    std::stack< std::pair<int, list_it_t> > dfsStack; //<vertex, neighbour index
    dfsStack.push( std::pair<int, list_it_t>(0, adj_lists[0].begin()) );
    parent[0] = 0;
    visited[0] = true;

    while(!dfsStack.empty())
    {
        std::pair<int, list_it_t> &currentVertex = dfsStack.top();

        if(currentVertex.second == adj_lists[currentVertex.first].end())
        {
            dfsStack.pop();
            continue;
        }

        while(currentVertex.second != adj_lists[currentVertex.first].end())
        {
            int neighbour = *currentVertex.second;
            currentVertex.second++;
            if(!visited[neighbour])
            {
                parent[neighbour] = currentVertex.first;
                visited[neighbour] = 1;
                dfsStack.push(std::pair<int, list_it_t>(neighbour, adj_lists[neighbour].begin()));
                break;
            }
        }
    }
}

//TODO LCA O(|V|log|V|)
void prune(const std::vector< std::list<int> >& adj_lists,
           const int setsCount, const int vertex_set[],
           std::vector< std::pair<int, int> > &forest_edges)
{
    const int verticesCount = adj_lists.size();
    const int logVerticesCount = ceil(log2(adj_lists.size()) + 2);
    int LCAjumps[logVerticesCount][verticesCount];
    memset(LCAjumps, 0, sizeof(int) * logVerticesCount * verticesCount);

    int parent[verticesCount];
    memset(parent, 0, sizeof(int) * verticesCount);
    getParents(adj_lists, parent);



    for(int i = 0; i < verticesCount; ++i)
    {
        std::cout << "vertex: " << i << " parent: " << parent[i] << std::endl;
    }

    for(int jmp = 1; jmp < logVerticesCount; ++jmp)
    {
        for(int i = 0; i < verticesCount; ++i)
        {
            LCAjumps[jmp][i] = LCAjumps[jmp-1][ LCAjumps[jmp-1][i] ];
        }
    }

    int setLCA[setsCount];
    int vertex_value[verticesCount];
    int setCardinality[setsCount];
    memset(setCardinality, 0, sizeof(int) * verticesCount);
    for(int i = 0; i < verticesCount; ++i)
    {
        if(vertex_set[i] != -1)
        {
            setLCA[vertex_set[i]] = i;
            setCardinality[vertex_set[i]] += 1;
            vertex_value[i] = 1;
        }
        else
        {
            vertex_value[i] = 0;
        }
    }

    //compute LCA for each set, change vertex_value for LCA
}

template <typename G, typename S, typename OutputIterator>
void SteinerForest(const G& graph, const S& sets, OutputIterator steiner_forest_edges)
{
    const typename boost::graph_traits<G>::vertices_size_type verticesCount = num_vertices(graph);
    int distances[verticesCount];
    memset(distances, 0, sizeof(int) * verticesCount);
    bool active[verticesCount];
    memset(active, 0, sizeof(bool) * verticesCount);

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
    int compoundTerminals[verticesCount][setsCount];
    memset(compoundTerminals, 0, sizeof(int) * verticesCount * setsCount);

    int setCardinality[setsCount];
    memset(setCardinality, 0, sizeof(int) * setsCount);

    //dsu
    typedef typename boost::property_map<G, boost::vertex_index_t>::type vertex_dsu_rank_map_t;
    vertex_dsu_rank_map_t v = get(boost::vertex_index, graph);
    int rank[verticesCount];
    memset(rank, 0, sizeof(int) * verticesCount);

    typedef boost::iterator_property_map<int *, vertex_dsu_rank_map_t, int, int&> dsu_rank_t;
    dsu_rank_t dsu_rank(rank, v);

    typedef typename boost::property_map<G, boost::vertex_index_t>::type vertex_dsu_parent_map_t;
    vertex_dsu_parent_map_t p = get(boost::vertex_index, graph);
    int parent[verticesCount];
    memset(parent, 0, sizeof(int) * verticesCount);

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
                         std::greater< queue_element_t > > edge_queue;

    vertex_iterator = vertices(graph);
    int vertex_set[verticesCount];
    memset(vertex_set, 0, sizeof(int) * verticesCount);
    while(vertex_iterator.first != vertex_iterator.second)
    {
        int set_id = get(sets, *vertex_iterator.first);
        vertex_set[*vertex_iterator.first] = set_id;
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

    typedef typename boost::graph_traits<G>::edge_descriptor edge_t;
    std::vector<edge_t> unpruned_forest_edges;
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
            unpruned_forest_edges.push_back(edge);

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


    std::cout << "edges start" << std::endl;
    std::vector< std::list<int> > adj_lists(verticesCount);
    for(auto it = unpruned_forest_edges.begin(); it != unpruned_forest_edges.end(); it++)
    {
        int s = source(*it, graph);
        int t = target(*it, graph);
        adj_lists[s].push_front(t);
        adj_lists[t].push_front(s);
    }


    std::vector< std::pair<int, int> > forest_edges;
    prune(adj_lists, setsCount, vertex_set, forest_edges);

    std::cout << "edges end" << std::endl;

    //pruning



    //write result edges
    //*steiner_forest_edges++ = edge;
}

#endif /* _STEINER_FOREST_H */
