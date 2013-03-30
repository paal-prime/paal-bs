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
 * @param vertices_count Number of vertices in graph.
 * @param vertex_value special precomputed vertices values that help find
 * essential edges.
 * @param forest_edges Output parameter that will store essential edges.
 **/
template <typename G>
void pick_forest_edges(
  const std::vector< std::list<typename G::vertex_t> >& adj_lists,
  const size_t vertices_count,
  const int vertex_value[],
  std::vector<typename G::edge_t>& forest_edges)
{
  typedef typename G::vertex_t vertex_t;
  typedef typename G::edge_t edge_t;

  bool visited[vertices_count];
  memset(visited, false, sizeof(visited[0]) * vertices_count);
  typedef typename std::list<vertex_t>::const_iterator list_it_t;
  std::stack< std::pair<vertex_t, list_it_t> > dfs_stack;

  int vertex_sum[vertices_count];
  memset(vertex_sum, 0, sizeof(vertex_sum[0]) * vertices_count);

  // XXX: Hidden dependency - we take advantage of fact that every
  // connected compound was rooted at vertex with the lowest number
  // when computing LCA
  for (size_t i = 0; i < vertices_count; ++i)
  {
    if (visited[i])
    {
      continue;
    }

    dfs_stack.push(std::pair<vertex_t, list_it_t>(i, adj_lists[i].begin()));

    visited[i] = true;

    while (!dfs_stack.empty())
    {
      std::pair<vertex_t, list_it_t> &current_vertex = dfs_stack.top();

      if (current_vertex.second == adj_lists[current_vertex.first].end())
      {
        vertex_sum[current_vertex.first] = vertex_value[current_vertex.first];
        for (list_it_t it = adj_lists[current_vertex.first].begin();
             it != adj_lists[current_vertex.first].end(); ++it)
        {
          vertex_sum[current_vertex.first] += vertex_sum[*it];
          if (vertex_sum[*it] > 0)
          {
            forest_edges.push_back(edge_t(current_vertex.first, *it));
          }
        }
        dfs_stack.pop();
        continue;
      }

      while (current_vertex.second != adj_lists[current_vertex.first].end())
      {
        vertex_t neighbour = *current_vertex.second;
        current_vertex.second++;
        if (!visited[neighbour])
        {
          visited[neighbour] = 1;
          dfs_stack.push(std::pair<vertex_t, list_it_t>(neighbour,
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
 * @param vertices_log First index of LCAjumps parameter.
 * @param vertices_count Second index of LCAjumps parameter.
 * @returns Label of vertex that is a LCA of input vertices.
 * */
template <typename G>
typename G::vertex_t lca(
  typename G::vertex_t a,
  typename G::vertex_t b,
  int lvl[],
  typename G::vertex_t *LCAjumps,
  const int vertices_log, const size_t vertices_count)
{
  if (lvl[a] > lvl[b])
  {
    std::swap(a, b);
  }

  int lvl_diff = lvl[b] - lvl[a];

  int lvl_index = 0;
  while (lvl_diff)
  {
    if (lvl_diff % 2)
    {
      b = LCAjumps[vertices_count * lvl_index + b];
    }
    lvl_diff /= 2;
    lvl_index += 1;
  }

  if (a == b)
  {
    return a;
  }

  for (int i = vertices_log - 1; i >= 0; i--)
  {
    if (LCAjumps[vertices_count * i + a] != LCAjumps[vertices_count * i + b])
    {
      a = LCAjumps[vertices_count * i + a];
      b = LCAjumps[vertices_count * i + b];
    }
  }

  return LCAjumps[a];
}

/**
 * @brief Given graph roots it's every compound component at vertex with
 * the lowest number also compute parents and distances from root (level)
 * for all vertices.
 * @param adj_lists Graph in form of adjacency lists.
 * @param vertices_count Number of vertices in graph.
 * @param parent Output parameter storing parent's labels for corresponding
 * vertices.
 * @param lvl Output parameter storing distances from root for corresponding
 * vertices.
 **/
template <typename G>
void get_parents(
  const std::vector< std::list<typename G::vertex_t> >& adj_lists,
  const size_t vertices_count,
  typename G::vertex_t parent[],
  int lvl[])
{
  typedef typename G::vertex_t vertex_t;

  bool visited[vertices_count];
  memset(visited, false, sizeof(visited[0]) * vertices_count);
  typedef typename std::list<vertex_t>::const_iterator list_it_t;
  std::stack< std::pair<vertex_t, list_it_t> > dfs_stack;

  for (size_t i = 0; i < vertices_count; ++i)
  {
    if (visited[i])
    {
      continue;
    }

    dfs_stack.push(std::pair<vertex_t, list_it_t>(i, adj_lists[i].begin()));
    parent[i] = i;
    lvl[i] = 1;
    visited[i] = true;

    while (!dfs_stack.empty())
    {
      std::pair<vertex_t, list_it_t> &current_vertex = dfs_stack.top();

      if (current_vertex.second == adj_lists[current_vertex.first].end())
      {
        dfs_stack.pop();
        continue;
      }

      while (current_vertex.second != adj_lists[current_vertex.first].end())
      {
        vertex_t neighbour = *current_vertex.second;
        current_vertex.second++;
        if (!visited[neighbour])
        {
          parent[neighbour] = current_vertex.first;
          visited[neighbour] = 1;
          dfs_stack.push(std::pair<vertex_t, list_it_t>(neighbour,
              adj_lists[neighbour].begin()));
          lvl[neighbour] = dfs_stack.size();
          break;
        }
      }
    }
  }
}

/**
 * @brief Removes useless edges from given unpruned Steiner Forest.
 * @param vertices_count Number of vertices in graph.
 * @param vertex_set Array that says whether some vertex v belongs to
 * some set of terminals or not and if so to which one.
 * @param unpruned_forest_edges Edges of unpruned Steiner Forest.
 * @param forest_edges Output parameter that will store edges of pruned forest.
 **/
template <typename G>
void prune(
  const size_t vertices_count,
  const int sets_count,
  const int vertex_set[],
  std::vector<typename G::weighted_edge_t>& unpruned_forest_edges,
  std::vector<typename G::edge_t> &forest_edges)
{
  typedef typename G::vertex_t vertex_t;

  const size_t log_vertices_count = ceil(log2(vertices_count) + 2);

  std::vector< std::list<vertex_t> > adj_lists(vertices_count);
  for (auto it = unpruned_forest_edges.begin();
       it != unpruned_forest_edges.end(); it++)
  {
    adj_lists[it->source].push_front(it->target);
    adj_lists[it->target].push_front(it->source);
  }

  vertex_t LCAjumps[log_vertices_count][vertices_count];
  memset(LCAjumps, 0, sizeof(LCAjumps[0][0]) *
         log_vertices_count * vertices_count);

  vertex_t parent[vertices_count];
  memset(parent, 0, sizeof(parent[0]) * vertices_count);
  int lvl[vertices_count];
  memset(lvl, 0, sizeof(lvl[0]) * vertices_count);

  // XXX: Hidden dependency - we take advantage of fact that every
  // connected compound is rooted at vertex with the lowest number
  // when computing LCA
  get_parents<G>(adj_lists, vertices_count, parent, lvl);

  for (size_t i = 0; i < vertices_count; ++i)
  {
    LCAjumps[0][i] = parent[i];
  }

  for (size_t jmp = 1; jmp < log_vertices_count; ++jmp)
  {
    for (size_t i = 0; i < vertices_count; ++i)
    {
      LCAjumps[jmp][i] = LCAjumps[jmp - 1][LCAjumps[jmp - 1][i]];
    }
  }

  vertex_t set_LCA[sets_count];
  int vertex_value[vertices_count];
  int set_cardinality[sets_count];
  memset(set_cardinality, 0, sizeof(set_cardinality[0]) * vertices_count);
  for (size_t i = 0; i < vertices_count; ++i)
  {
    if (vertex_set[i] != -1)
    {
      set_LCA[vertex_set[i]] = i;
      set_cardinality[vertex_set[i]] += 1;
      vertex_value[i] = 1;
    }
    else
    {
      vertex_value[i] = 0;
    }
  }

  for (size_t i = 0; i < vertices_count; ++i)
  {
    if (vertex_set[i] != -1)
    {
      set_LCA[vertex_set[i]] = lca<G>(set_LCA[vertex_set[i]], i, lvl,
          LCAjumps[0], log_vertices_count,
          vertices_count);
    }
  }

  for (int i = 0; i < sets_count; ++i)
  {
    vertex_value[set_LCA[i]] -= set_cardinality[i];
  }

  pick_forest_edges<G>(adj_lists, vertices_count, vertex_value, forest_edges);
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
  const int sets_count,
  std::vector<typename G::weighted_edge_t>& unpruned_forest_edges)
{
  typedef typename G::edge_iterator_t edge_iterator_t;
  typedef typename G::vertex_t vertex_t;
  typedef typename G::edge_weight_t edge_weight_t;
  typedef typename G::edge_t edge_t;

  const size_t vertices_count = graph.get_vertices_count();
  edge_weight_t distances[vertices_count];
  bool active[vertices_count];
  for (typename G::vertex_t v = 0; v < vertices_count; ++v)
  {
    distances[v] = edge_weight_t();
    active[v] = false;
  }

  int set_cardinality[sets_count];
  memset(set_cardinality, 0, sizeof(set_cardinality[0]) * sets_count);
  for (vertex_t v = 0; v < vertices_count; ++v)
  {
    int set_id = vertex_set[v];
    if (set_id != -1)
    {
      set_cardinality[set_id] += 1;
    }
  }

  int active_sets = 0;
  int set_cardinality_counter[vertices_count];
  memset(set_cardinality_counter, 0, sizeof(set_cardinality[0]) * vertices_count);
  boost::heap::skew_heap<int> compound_terminals[vertices_count];
  for (vertex_t v = 0; v < vertices_count; ++v)
  {
    int set_id = vertex_set[v];
    if (set_id != -1 && set_cardinality[set_id] != 1)
    {
      active_sets += 1;
      active[v] = 1;
      compound_terminals[v].push(set_id);
    }
  }

  int rank[vertices_count];
  memset(rank, 0, sizeof(rank[0]) * vertices_count);
  typedef boost::iterator_property_map < int *, boost::identity_property_map,
          int, int& > dsu_rank_t;
  dsu_rank_t dsu_rank(rank, boost::identity_property_map());

  vertex_t parent[vertices_count];
  memset(parent, 0, sizeof(parent[0]) * vertices_count);

  typedef boost::iterator_property_map < vertex_t *,
          boost::identity_property_map, vertex_t, vertex_t& > dsu_parent_t;
  dsu_parent_t dsu_parent(parent, boost::identity_property_map());
  boost::disjoint_sets<dsu_rank_t, dsu_parent_t> dsu(dsu_rank, dsu_parent);

  for (vertex_t v = 0; v < vertices_count; ++v)
  {
    dsu.make_set(v);
  }

  while (active_sets)
  {
    edge_weight_t weight = edge_weight_t();
    edge_t edge;
    bool candidate_initialized = false;

    for (vertex_t v = 0; v < vertices_count; ++v)
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

            if (!candidate_initialized || candidate < weight)
            {
              weight = candidate;
              edge.source = source;
              edge.target = target;
              candidate_initialized = true;
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

    for (vertex_t v = 0; v < vertices_count; ++v)
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
    compound_terminals[new_root].merge(compound_terminals[merge]);

    if (active[source_parent] && active[target_parent])
    {
      active_sets -= 1;
      active[new_root] = 1;

      while (compound_terminals[new_root].size() >= 2)
      {
        int a = compound_terminals[new_root].top();
        compound_terminals[new_root].pop();
        int b = compound_terminals[new_root].top();

        if (a == b)
        {
          set_cardinality_counter[a] += 1;
        }
        else
        {
          compound_terminals[new_root].push(a);
          break;
        }

        if (set_cardinality_counter[a] + 1 == set_cardinality[a])
        {
          compound_terminals[new_root].pop();
        }
      }

      if (compound_terminals[new_root].empty())
      {
        active_sets -= 1;
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
void steiner_forest(
  const G& graph,
  const int vertex_set[],
  OutputIterator steiner_forest_edges)
{
  typedef typename G::edge_iterator_t edge_iterator_t;
  typedef typename G::vertex_t vertex_t;
  typedef typename G::edge_weight_t edge_weight_t;
  typedef typename G::weighted_edge_t weighted_edge_t;
  typedef typename G::edge_t edge_t;

  const size_t vertices_count = graph.get_vertices_count();
  if (vertices_count == 0)
  {
    return;
  }

  const int max_set_number = *std::max_element(vertex_set,
      vertex_set + vertices_count);
  const int sets_count = max_set_number + 1;
  if (sets_count == 0)
  {
    return;
  }

  std::vector<weighted_edge_t> unpruned_forest_edges;
  unpruned_forest(graph, vertex_set, sets_count, unpruned_forest_edges);

  std::vector<edge_t> forest_edges;
  prune<G>(vertices_count, sets_count, vertex_set, unpruned_forest_edges,
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
