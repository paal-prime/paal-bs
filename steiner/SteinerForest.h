#ifndef STEINER_STEINERFOREST_H_
#define STEINER_STEINERFOREST_H_

#include <boost/pending/disjoint_sets.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/heap/skew_heap.hpp>
#include <queue>
#include <set>
#include <vector>
#include <list>
#include <stack>
#include <functional>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <utility>

/**
 * @brief Picks edges that Steiner Forest must contain given unpruned forest.
 * @param adj_lists Graph in form of adjacency lists.
 * @param verticesCount Number of vertices in graph.
 * @param vertex_value special precomputed vertices values that help find
 * essential edges.
 * @param forest_edges Output parameter that will store essential edges.
 **/
template <typename G>
void pick_forest_edges(
  const std::vector< std::list<typename G::vertex_t> >& adj_lists,
  const size_t verticesCount,
  const int vertex_value[],
  std::vector<typename G::edge_t>& forest_edges)
{
  typedef typename G::vertex_t vertex_t;
  typedef typename G::edge_t edge_t;

  bool visited[verticesCount];
  memset(visited, false, sizeof(visited[0]) * verticesCount);
  typedef typename std::list<vertex_t>::const_iterator list_it_t;
  std::stack< std::pair<vertex_t, list_it_t> > dfsStack;

  int vertex_sum[verticesCount];
  memset(vertex_sum, 0, sizeof(vertex_sum[0]) * verticesCount);

  // XXX: Hidden dependency - we take advantage of fact that every
  // connected compound was rooted at vertex with the lowest number
  // when computing LCA
  for (size_t i = 0; i < verticesCount; ++i)
  {
    if (visited[i])
    {
      continue;
    }

    dfsStack.push(std::pair<vertex_t, list_it_t>(i, adj_lists[i].begin()));

    visited[i] = true;

    while (!dfsStack.empty())
    {
      std::pair<vertex_t, list_it_t> &currentVertex = dfsStack.top();

      if (currentVertex.second == adj_lists[currentVertex.first].end())
      {
        vertex_sum[currentVertex.first] = vertex_value[currentVertex.first];
        for (list_it_t it = adj_lists[currentVertex.first].begin();
             it != adj_lists[currentVertex.first].end(); ++it)
        {
          vertex_sum[currentVertex.first] += vertex_sum[*it];
          if (vertex_sum[*it] > 0)
          {
            forest_edges.push_back(edge_t(currentVertex.first, *it));
          }
        }
        dfsStack.pop();
        continue;
      }

      while (currentVertex.second != adj_lists[currentVertex.first].end())
      {
        vertex_t neighbour = *currentVertex.second;
        currentVertex.second++;
        if (!visited[neighbour])
        {
          visited[neighbour] = 1;
          dfsStack.push(std::pair<vertex_t, list_it_t>(neighbour,
              adj_lists[neighbour].begin()));
          break;
        }
      }
    }
  }
}

/**
 * @brief Computes lowest common ancestor (LCA) for given two vertices.
 * @param a First vertex.
 * @param b Second vertex.
 * @param lvl Array storing distances from root of corresponding vertices.
 * @param LCAjumps Two dimensional array storing 2^i parent of corresponding
 * vertices.
 * @param verticesLog First index of LCAjumps parameter.
 * @param verticesCount Second index of LCAjumps parameter.
 * @returns Label of vertex that is a LCA of input vertices.
 * */
template <typename G>
typename G::vertex_t lca(
  typename G::vertex_t a,
  typename G::vertex_t b,
  int lvl[],
  typename G::vertex_t *LCAjumps,
  const int verticesLog, const size_t verticesCount)
{
  if (lvl[a] > lvl[b])
  {
    std::swap(a, b);
  }

  int lvlDiff = lvl[b] - lvl[a];

  int lvlIndex = 0;
  while (lvlDiff)
  {
    if (lvlDiff % 2)
    {
      b = LCAjumps[verticesCount * lvlIndex + b];
    }
    lvlDiff /= 2;
    lvlIndex += 1;
  }

  if (a == b)
  {
    return a;
  }

  for (int i = verticesLog - 1; i >= 0; i--)
  {
    if (LCAjumps[verticesCount * i + a] != LCAjumps[verticesCount * i + b])
    {
      a = LCAjumps[verticesCount * i + a];
      b = LCAjumps[verticesCount * i + b];
    }
  }

  return LCAjumps[a];
}

/**
 * @brief Given graph roots it's every compound component at vertex with
 * the lowest number also compute parents and distances from root (level)
 * for all vertices.
 * @param adj_lists Graph in form of adjacency lists.
 * @param verticesCount Number of vertices in graph.
 * @param parent Output parameter storing parent's labels for corresponding
 * vertices.
 * @param lvl Output parameter storing distances from root for corresponding
 * vertices.
 **/
template <typename G>
void getParents(
  const std::vector< std::list<typename G::vertex_t> >& adj_lists,
  const size_t verticesCount,
  typename G::vertex_t parent[],
  int lvl[])
{
  typedef typename G::vertex_t vertex_t;

  bool visited[verticesCount];
  memset(visited, false, sizeof(visited[0]) * verticesCount);
  typedef typename std::list<vertex_t>::const_iterator list_it_t;
  std::stack< std::pair<vertex_t, list_it_t> > dfsStack;

  for (size_t i = 0; i < verticesCount; ++i)
  {
    if (visited[i])
    {
      continue;
    }

    dfsStack.push(std::pair<vertex_t, list_it_t>(i, adj_lists[i].begin()));
    parent[i] = i;
    lvl[i] = 1;
    visited[i] = true;

    while (!dfsStack.empty())
    {
      std::pair<vertex_t, list_it_t> &currentVertex = dfsStack.top();

      if (currentVertex.second == adj_lists[currentVertex.first].end())
      {
        dfsStack.pop();
        continue;
      }

      while (currentVertex.second != adj_lists[currentVertex.first].end())
      {
        vertex_t neighbour = *currentVertex.second;
        currentVertex.second++;
        if (!visited[neighbour])
        {
          parent[neighbour] = currentVertex.first;
          visited[neighbour] = 1;
          dfsStack.push(std::pair<vertex_t, list_it_t>(neighbour,
              adj_lists[neighbour].begin()));
          lvl[neighbour] = dfsStack.size();
          break;
        }
      }
    }
  }
}

/**
 * @brief Removes useless edges from given unpruned Steiner Forest.
 * @param verticesCount Number of vertices in graph.
 * @param vertex_set Array that says whether some vertex v belongs to
 * some set of terminals or not and if so to which one.
 * @param unpruned_forest_edges Edges of unpruned Steiner Forest.
 * @param forest_edges Output parameter that will store edges of pruned forest.
 **/
template <typename G>
void prune(
  const size_t verticesCount,
  const int setsCount,
  const int vertex_set[],
  std::vector<typename G::weighted_edge_t>& unpruned_forest_edges,
  std::vector<typename G::edge_t> &forest_edges)
{
  typedef typename G::vertex_t vertex_t;

  const size_t logVerticesCount = ceil(log2(verticesCount) + 2);

  std::vector< std::list<vertex_t> > adj_lists(verticesCount);
  for (auto it = unpruned_forest_edges.begin();
       it != unpruned_forest_edges.end(); it++)
  {
    adj_lists[it->source].push_front(it->target);
    adj_lists[it->target].push_front(it->source);
  }

  vertex_t LCAjumps[logVerticesCount][verticesCount];
  memset(LCAjumps, 0, sizeof(LCAjumps[0][0]) *
         logVerticesCount * verticesCount);

  vertex_t parent[verticesCount];
  memset(parent, 0, sizeof(parent[0]) * verticesCount);
  int lvl[verticesCount];
  memset(lvl, 0, sizeof(lvl[0]) * verticesCount);

  // XXX: Hidden dependency - we take advantage of fact that every
  // connected compound is rooted at vertex with the lowest number
  // when computing LCA
  getParents<G>(adj_lists, verticesCount, parent, lvl);

  for (size_t i = 0; i < verticesCount; ++i)
  {
    LCAjumps[0][i] = parent[i];
  }

  for (size_t jmp = 1; jmp < logVerticesCount; ++jmp)
  {
    for (size_t i = 0; i < verticesCount; ++i)
    {
      LCAjumps[jmp][i] = LCAjumps[jmp - 1][LCAjumps[jmp - 1][i]];
    }
  }

  vertex_t setLCA[setsCount];
  int vertex_value[verticesCount];
  int setCardinality[setsCount];
  memset(setCardinality, 0, sizeof(setCardinality[0]) * verticesCount);
  for (size_t i = 0; i < verticesCount; ++i)
  {
    if (vertex_set[i] != -1)
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

  for (size_t i = 0; i < verticesCount; ++i)
  {
    if (vertex_set[i] != -1)
    {
      setLCA[vertex_set[i]] = lca<G>(setLCA[vertex_set[i]], i, lvl,
          LCAjumps[0], logVerticesCount,
          verticesCount);
    }
  }

  for (int i = 0; i < setsCount; ++i)
  {
    vertex_value[setLCA[i]] -= setCardinality[i];
  }

  pick_forest_edges<G>(adj_lists, verticesCount, vertex_value, forest_edges);
}

/**
 * @brief Finds unpruned Steiner Forest (forest that might contain useless
 * edges that will be removed (pruned) later.
 * @param graph Weighted undirected graph.
 * @param vertex_set Array that says whether given vertex v belongs to some
 * set of terminals or not and if so to which one.
 * @param unpruned_forest_edges Output parameter that will store edges of
 * found unpruned forest.
 * */
// TODO(lukasz): Improve time complexity.
template <typename G>
void unpruned_forest(
  const G& graph, const int vertex_set[],
  const int setsCount,
  std::vector<typename G::weighted_edge_t>& unpruned_forest_edges)
{
  typedef typename G::edge_iterator_t edge_iterator_t;
  typedef typename G::vertex_t vertex_t;
  typedef typename G::edge_weight_t edge_weight_t;
  typedef typename G::edge_t edge_t;

  const size_t verticesCount = graph.verticesCount();
  edge_weight_t distances[verticesCount];
  bool active[verticesCount];
  for (typename G::vertex_t v = 0; v < verticesCount; ++v)
  {
    distances[v] = edge_weight_t();
    active[v] = false;
  }

  int setCardinality[setsCount];
  memset(setCardinality, 0, sizeof(setCardinality[0]) * setsCount);
  for (vertex_t v = 0; v < verticesCount; ++v)
  {
    int set_id = vertex_set[v];
    if (set_id != -1)
    {
      setCardinality[set_id] += 1;
    }
  }

  int activeSets = 0;
  int setCardinalityCounter[verticesCount];
  memset(setCardinalityCounter, 0, sizeof(setCardinality[0]) * verticesCount);
  boost::heap::skew_heap<int> compoundTerminals[verticesCount];
  for (vertex_t v = 0; v < verticesCount; ++v)
  {
    int set_id = vertex_set[v];
    if (set_id != -1 && setCardinality[set_id] != 1)
    {
      activeSets += 1;
      active[v] = 1;
      compoundTerminals[v].push(set_id);
    }
  }

  int rank[verticesCount];
  memset(rank, 0, sizeof(rank[0]) * verticesCount);
  typedef boost::iterator_property_map < int *, boost::identity_property_map,
          int, int& > dsu_rank_t;
  dsu_rank_t dsu_rank(rank, boost::identity_property_map());

  vertex_t parent[verticesCount];
  memset(parent, 0, sizeof(parent[0]) * verticesCount);

  typedef boost::iterator_property_map < vertex_t *,
          boost::identity_property_map, vertex_t, vertex_t& > dsu_parent_t;
  dsu_parent_t dsu_parent(parent, boost::identity_property_map());
  boost::disjoint_sets<dsu_rank_t, dsu_parent_t> dsu(dsu_rank, dsu_parent);

  for (vertex_t v = 0; v < verticesCount; ++v)
  {
    dsu.make_set(v);
  }

  while (activeSets)
  {
    edge_weight_t weight = edge_weight_t();
    edge_t edge;
    bool candidateInitialized = false;

    for (vertex_t v = 0; v < verticesCount; ++v)
    {
      edge_iterator_t it;
      edge_iterator_t end;
      boost::tie(it, end) = graph.out_edges(v);
      while (it != end)
      {
        vertex_t source = v;
        vertex_t target = (*it).target;

        vertex_t source_parent = dsu.find_set(source);
        vertex_t target_parent = dsu.find_set(target);
        if (source_parent != target_parent)
        {
          if (active[source_parent] || active[target_parent])
          {
            edge_weight_t candidate = 2 * ((*it).weight
                - distances[source]
                - distances[target]);

            if (active[source_parent] && active[target_parent])
            {
              candidate = candidate / 2;
            }

            if (!candidateInitialized || candidate < weight)
            {
              weight = candidate;
              edge.source = source;
              edge.target = target;
              candidateInitialized = true;
            }
          }
        }

        it++;
      }
    }

    vertex_t source = edge.source;
    vertex_t target = edge.target;

    unpruned_forest_edges.push_back(typename G::weighted_edge_t(source,
        target,
        weight));

    for (vertex_t v = 0; v < verticesCount; ++v)
    {
      if (active[dsu.find_set(v)])
      {
        distances[v] += weight;
      }
    }

    vertex_t source_parent = dsu.find_set(source);
    vertex_t target_parent = dsu.find_set(target);

    dsu.link(source, target);
    vertex_t new_root = dsu.find_set(source);

    size_t merge = (new_root == source_parent) ? target_parent :
        source_parent;
    compoundTerminals[new_root].merge(compoundTerminals[merge]);

    if (active[source_parent] && active[target_parent])
    {
      activeSets -= 1;
      active[new_root] = 1;

      while (compoundTerminals[new_root].size() >= 2)
      {
        int a = compoundTerminals[new_root].top();
        compoundTerminals[new_root].pop();
        int b = compoundTerminals[new_root].top();

        if (a == b)
        {
          setCardinalityCounter[a] += 1;
        }
        else
        {
          compoundTerminals[new_root].push(a);
          break;
        }

        if (setCardinalityCounter[a] + 1 == setCardinality[a])
        {
          compoundTerminals[new_root].pop();
        }
      }

      if (compoundTerminals[new_root].empty())
      {
        activeSets -= 1;
        active[new_root] = 0;
      }
    }
    else
    {
      active[new_root] = 1;
    }
  }
}

/**
 * @brief Given set of unweighted edges retrives original weight for each edge
 * and store them in output parameter.
 * @param Input weighted undirected graph.
 * @param forest_edges Set of edges without weight information.
 * @param steiner_forest_edges Output iterator used to store result.
 * */
template <typename G, typename OutputIterator>
void retrieve_weighted_forest_edges(
  const G& graph,
  std::vector<typename G::edge_t> forest_edges,
  OutputIterator steiner_forest_edges)
{
  typedef typename G::edge_iterator_t edge_iterator_t;
  typedef typename G::vertex_t vertex_t;
  typedef typename G::edge_weight_t edge_weight_t;
  typedef typename G::weighted_edge_t weighted_edge_t;

  size_t current_edge = 0;
  while (current_edge < forest_edges.size())
  {
    vertex_t source = forest_edges[current_edge].source;

    edge_iterator_t it, end;
    boost::tie(it, end) = graph.out_edges(source);

    while (it != end)
    {
      if (forest_edges[current_edge].target == (*it).target)
      {
        *steiner_forest_edges++ = weighted_edge_t(source,
            (*it).target, (*it).weight);
        current_edge += 1;

        if (current_edge >= forest_edges.size())
        {
          break;
        }
      }

      if (forest_edges[current_edge].source != source)
      {
        break;
      }

      it++;
    }
  }
}

/**
 * @brief Finds Steiner Forest for given weighted undirected graph and set of
 * terminals.
 * @param graph Weighted undirected graph,
 * @param vertex_set It's an array that says whether vertex v is a terminal and
 * if so to which set it belongs. If vertex_set[v] == -1 then v is not a
 * terminal otherwise vertex_set[v] is corresponding set's number.
 * @param steiner_forest_edges Output iterator used to store edges of
 * Steiner Forest.
 **/
template <typename G, typename OutputIterator>
void SteinerForest(
  const G& graph,
  const int vertex_set[],
  OutputIterator steiner_forest_edges)
{
  typedef typename G::edge_iterator_t edge_iterator_t;
  typedef typename G::vertex_t vertex_t;
  typedef typename G::edge_weight_t edge_weight_t;
  typedef typename G::weighted_edge_t weighted_edge_t;
  typedef typename G::edge_t edge_t;

  const size_t verticesCount = graph.verticesCount();
  if (verticesCount == 0)
  {
    return;
  }

  const int maxSetNumber = *std::max_element(vertex_set,
      vertex_set + verticesCount);
  const int setsCount = maxSetNumber + 1;
  if (setsCount == 0)
  {
    return;
  }

  std::vector<weighted_edge_t> unpruned_forest_edges;
  unpruned_forest(graph, vertex_set, setsCount, unpruned_forest_edges);

  std::vector<edge_t> forest_edges;
  prune<G>(verticesCount, setsCount, vertex_set, unpruned_forest_edges,
      forest_edges);

  for (size_t i = 0; i < forest_edges.size(); ++i)
  {
    if (forest_edges[i].source > forest_edges[i].target)
    {
      std::swap(forest_edges[i].source, forest_edges[i].target);
    }
  }
  std::sort(forest_edges.begin(), forest_edges.end());

  retrieve_weighted_forest_edges(graph, forest_edges, steiner_forest_edges);
}

#endif  // STEINER_STEINERFOREST_H_
