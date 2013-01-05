#ifndef _STEINER_FOREST_H
#define _STEINER_FOREST_H

#include <boost/pending/disjoint_sets.hpp>
#include <boost/property_map/property_map.hpp>
#include <queue>
#include <vector>
#include <list>
#include <stack>
#include <functional>
#include <cmath>
#include <cstring>
#include <algorithm>

//DEBUG
#include <iostream>

void getForestEdges(const std::vector< std::list<int> >& adj_lists, const int vertex_value[],
                    std::vector< std::pair<size_t, size_t> >& forest_edges)
{
    const int verticesCount = adj_lists.size();
    bool visited[verticesCount];
    memset(visited, false, sizeof(bool) * verticesCount);
    typedef std::list<int>::const_iterator list_it_t;
    std::stack< std::pair<int, list_it_t> > dfsStack; //<vertex, neighbour index

    int vertex_sum[verticesCount];
    memset(vertex_sum, 0, sizeof(int) * verticesCount);

    for(int i = 0; i < verticesCount; ++i)
    {
        if(visited[i])
        {
            continue;
        }

        dfsStack.push( std::pair<int, list_it_t>(i, adj_lists[i].begin()) );

        visited[i] = true;

        while(!dfsStack.empty())
        {
            std::pair<int, list_it_t> &currentVertex = dfsStack.top();

            if(currentVertex.second == adj_lists[currentVertex.first].end())
            {
                vertex_sum[currentVertex.first] = vertex_value[currentVertex.first];
                for(list_it_t it = adj_lists[currentVertex.first].begin();
                    it != adj_lists[currentVertex.first].end(); ++it)
                {
                    vertex_sum[currentVertex.first] += vertex_sum[*it];
                    if(vertex_sum[*it] > 0)
                    {
                        forest_edges.push_back(std::pair<size_t, size_t>(currentVertex.first, *it));
                    }
                }
                dfsStack.pop();
                continue;
            }

            while(currentVertex.second != adj_lists[currentVertex.first].end())
            {
                int neighbour = *currentVertex.second;
                currentVertex.second++;
                if(!visited[neighbour])
                {
                    visited[neighbour] = 1;
                    dfsStack.push(std::pair<int, list_it_t>(neighbour, adj_lists[neighbour].begin()));
                    break;
                }
            }
        }
    }
}

int lca(int a, int b, int lvl[], int *LCAjumps, const int verticesLog, const int verticesCount)
{
    if(lvl[a] > lvl[b])
    {
        std::swap(a, b);
    }

    int lvlDiff = lvl[b] - lvl[a];

    int k = 0;
    while(lvlDiff)
    {
        if(lvlDiff % 2)
        {
            b = LCAjumps[verticesCount*k + b];
        }
        lvlDiff /= 2;
        k += 1;
    }

    if(a == b)
    {
        return a;
    }

    for(int i = verticesLog - 1; i >= 0; i--)
    {
        if(LCAjumps[verticesCount*i + a] != LCAjumps[verticesCount*i + b])
        {
            a = LCAjumps[verticesCount*i + a];
            b = LCAjumps[verticesCount*i + b];
        }
    }

    return LCAjumps[a];
}

void getParents(const std::vector< std::list<int> >& adj_lists, int parent[], int lvl[])
{
    const int verticesCount = adj_lists.size();
    bool visited[verticesCount];
    memset(visited, false, sizeof(bool) * verticesCount);
    typedef std::list<int>::const_iterator list_it_t;
    std::stack< std::pair<int, list_it_t> > dfsStack; //<vertex, neighbour index

    for(int i = 0; i < verticesCount; ++i)
    {
        if(visited[i])
        {
            continue;
        }

        dfsStack.push( std::pair<int, list_it_t>(i, adj_lists[i].begin()) );
        parent[i] = i;
        lvl[i] = 1;
        visited[i] = true;

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
                    lvl[neighbour] = dfsStack.size();
                    break;
                }
            }
        }
    }
}

void prune(const std::vector< std::list<int> >& adj_lists,
           const int setsCount, const int vertex_set[],
           std::vector< std::pair<size_t, size_t> > &forest_edges)
{
    const int verticesCount = adj_lists.size();
    const int logVerticesCount = ceil(log2(adj_lists.size()) + 2);
    int LCAjumps[logVerticesCount][verticesCount];
    memset(LCAjumps, 0, sizeof(int) * logVerticesCount * verticesCount);

    int parent[verticesCount];
    int lvl[verticesCount];
    memset(parent, 0, sizeof(int) * verticesCount);
    memset(lvl, 0, sizeof(int) * verticesCount);
    getParents(adj_lists, parent, lvl);

    for(int i = 0; i < verticesCount; ++i)
    {
        LCAjumps[0][i] = parent[i];
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

    for(int i = 0; i < verticesCount; ++i)
    {
        if(vertex_set[i] != -1)
        {
            setLCA[vertex_set[i]] = lca(setLCA[vertex_set[i]], i, lvl, LCAjumps[0],
                                        logVerticesCount, verticesCount);
        }
    }

    for(int i = 0; i < setsCount; ++i)
    {
        vertex_value[setLCA[i]] -= setCardinality[i];
    }

    getForestEdges(adj_lists, vertex_value, forest_edges);
}


template <typename G, typename OutputIterator>
void SteinerForest(const G& graph, const int sets[], OutputIterator steiner_forest_edges)
{
    size_t verticesCount = graph.verticesCount();
    typename G::edge_weight_t distances[verticesCount];
    bool active[verticesCount];

    int maxSetNumber = -1;
    for(typename G::vertex_t v = 0; v < verticesCount; ++v)
    {
        maxSetNumber = std::max(maxSetNumber, sets[v]);
        distances[v] = typename G::edge_weight_t();
        active[v] = false;
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

    int rank[verticesCount];
    memset(rank, 0, sizeof(int) * verticesCount);

    typedef boost::iterator_property_map<int *, boost::identity_property_map, int, int&> dsu_rank_t;
    dsu_rank_t dsu_rank(rank, boost::identity_property_map());

    int parent[verticesCount];
    memset(parent, 0, sizeof(int) * verticesCount);

    typedef boost::iterator_property_map<int *, boost::identity_property_map, int, int&> dsu_parent_t;
    dsu_parent_t dsu_parent(parent, boost::identity_property_map());

    boost::disjoint_sets<dsu_rank_t, dsu_parent_t> dsu(dsu_rank, dsu_parent);

    int activeSets = 0;
    typedef typename G::edge_iterator_t edge_iterator_t;
    typedef typename G::vertex_t vertex_t;
    typedef typename G::edge_weight_t edge_weight_t;
    typedef std::pair<edge_weight_t, std::pair<vertex_t, vertex_t> > queue_element_t;
    std::priority_queue< queue_element_t,
                         std::vector< queue_element_t >,
                         std::greater< queue_element_t > > edge_queue;

    int vertex_set[verticesCount];
    memset(vertex_set, 0, sizeof(int) * verticesCount);
    for(vertex_t v = 0; v < verticesCount; ++v)
    {
        int set_id = sets[v];
        vertex_set[v] = set_id;
        if(set_id != -1)
        {
            setCardinality[set_id] += 1;
        }
    }

    for(vertex_t v = 0; v < verticesCount; ++v)
    {
        dsu.make_set(v);
        int set_id = sets[v];
        if(set_id != -1 && setCardinality[set_id] != 1)
        {
            activeSets += 1;
            active[v] = 1;
            distances[v] = edge_weight_t();
            compoundTerminals[v][set_id] += 1;

            //iterate over edges
            edge_iterator_t first;
            edge_iterator_t last;
            boost::tie(first, last) = graph.out_edges(v);
            while(first != last)
            {
                vertex_t source = v;
                vertex_t target = (*first).first;
                int weight = 0; //TODO type
                if(active[target])
                {
                    weight = (distances[target] + distances[source])/2 +
                              (*first).second;
                }
                else
                {
                    weight = distances[source] + 2*(*first).second;
                }

                edge_queue.push(std::make_pair(weight, std::make_pair(v, (*first).first)));

                first++;
            }
        }
    }

    std::vector<std::pair< std::pair<vertex_t, vertex_t>, edge_weight_t > > unpruned_forest_edges;
    while(activeSets)
    {
        int weight = 0; //TODO type
        std::pair<vertex_t, vertex_t> edge;
        assert(!edge_queue.empty());
        boost::tie(weight, edge) = edge_queue.top();

        edge_queue.pop();
        vertex_t source = edge.first;
        vertex_t target = edge.second;

        vertex_t source_parent = dsu.find_set(source);
        vertex_t target_parent = dsu.find_set(target);

        if(source_parent != target_parent)
        {
            unpruned_forest_edges.push_back(std::make_pair(edge, weight));

            dsu.link(source, target);
            vertex_t new_root = dsu.find_set(source);
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
                vertex_t parent = source;
                //copy paste - add adjacent edges, modified (parent)
                edge_iterator_t first;
                edge_iterator_t last;
                boost::tie(first, last) = graph.out_edges(target);
    //STOPPED HERE
                while(first != last)
                {
                    vertex_t source = target;
                    vertex_t target = (*first).first;
                    if(source != parent)
                    {
                        int weight = 0; //TODO type
                        if(active[target])
                        {
                            weight = (distances[target] + distances[source])/2 +
                                      (*first).second;
                        }
                        else
                        {
                            weight = distances[source] + 2*(*first).second;
                        }

                        edge_queue.push(std::make_pair(weight, std::make_pair(source, target)));
                    }
                    first++;
                }
                //end copy paste
            }
        }
    }
    std::vector< std::list<int> > adj_lists(verticesCount);
    for(auto it = unpruned_forest_edges.begin(); it != unpruned_forest_edges.end(); it++)
    {
        int s = ((*it).first).first;
        int t = ((*it).first).second;
        adj_lists[s].push_front(t);
        adj_lists[t].push_front(s);
    }

    std::vector< std::pair<vertex_t, vertex_t> > forest_edges;
    prune(adj_lists, setsCount, vertex_set, forest_edges);

    for(size_t i = 0; i < forest_edges.size(); ++i)
    {
        if(forest_edges[i].first > forest_edges[i].second)
        {
            std::swap(forest_edges[i].first, forest_edges[i].second);
        }
    }

    std::sort(forest_edges.begin(), forest_edges.end());

    size_t current_edge = 0;
    while(current_edge < forest_edges.size())
    {
        edge_iterator_t first, last;
        vertex_t source = forest_edges[current_edge].first;
        boost::tie(first, last) = graph.out_edges(source);

        while(first != last)
        {
            if(forest_edges[current_edge].first != source)
            {
                break;
            }

            if(forest_edges[current_edge].second == (*first).first)
            {
                *steiner_forest_edges++ = std::make_pair(std::make_pair(source, (*first).first),
                                                         (*first).second);
                current_edge += 1;
                if(current_edge >= forest_edges.size())
                {
                    break;
                }
            }

            first++;

        }
    }
}

#endif /* _STEINER_FOREST_H */
