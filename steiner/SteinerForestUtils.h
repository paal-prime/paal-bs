#ifndef STEINER_STEINERFORESTUTILS_H_
#define STEINER_STEINERFORESTUTILS_H_

#include <map>
#include <vector>
#include <algorithm>
#include <utility>
#include <list>
#include <set>
#include <stack>

#include "graph/AdjacencyMatrix.h"
#include "graph/AdjacencyLists.h"

namespace steiner
{
  /**
   * @brief computes cost of Steiner Forest Problem solution.
   */
  template<typename G>
  double fitness(std::vector<typename G::weighted_edge_t>& solution)
  {
    double cost(0);

    for (size_t i = 0; i < solution.size(); ++i)
    {
      cost += solution[i].weight;
    }

    return cost;
  }

  /**
   * @brief Depth first search used to compute graph's connected components.
   * @param v starting vertex.
   * @param graph traversed graph.
   * @param current_connected_component index of currently traversed
   * connected component.
   * @param visited array storing information whether vertex was already
   * visited during search.
   * @param connected_component array storing information to which
   * connected component vertex belongs.
   */
  template<typename GRAPH>
  void dfs(size_t v, const GRAPH& graph, size_t current_connected_component,
      std::vector<bool>& visited, std::vector<int>& connected_component)
  {
    visited[v] = true;
    connected_component[v] = current_connected_component;

    auto it = graph.out_edges(v);

    while (it.first != it.second)
    {
      size_t neighbour = *it.first;
      if (!visited[neighbour])
      {
        dfs(neighbour, graph, current_connected_component, visited,
            connected_component);
      }
      it.first++;
    }
  }

  /**
   * @brief checks whether given solution is feasible.
   * That means each pair of vertices belonging to the same set
   * is connected.
   */
  template<typename G>
  bool is_feasible_solution(const G& graph, const int vertex_set[],
      std::vector<typename G::weighted_edge_t>& solution)
  {
    const size_t vertices_count = graph.get_vertices_count();
    std::vector<int> connected_component(vertices_count, -1);
    std::vector<bool> visited(vertices_count, false);

    const int max_set_number = *std::max_element(vertex_set,
        vertex_set + vertices_count);
    const size_t sets_count = max_set_number + 1;

    std::vector<int> set[sets_count];

    for (size_t i = 0; i < vertices_count; ++i)
    {
      if (vertex_set[i] != -1)
      {
        set[vertex_set[i]].push_back(i);
      }
    }

    graph::AdjacencyMatrix<graph::undirected, graph::unweighted>
    subgraph(vertices_count);

    for (size_t i = 0; i < solution.size(); ++i)
    {
      subgraph.add_edge(solution[i].source, solution[i].target);
    }

    int current_connected_component = 0;

    for (size_t i = 0; i < vertices_count; ++i)
    {
      if (!visited[i])
      {
        dfs(i, subgraph, current_connected_component, visited,
            connected_component);
        current_connected_component++;
      }
    }

    for (size_t i = 0; i < sets_count; ++i)
    {
      for (size_t j = 1; j < set[i].size(); ++j)
      {
        if (connected_component[set[i][j]] != connected_component[set[i][0]])
        {
          return false;
        }
      }
    }

    return true;
  }

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
   * @param sets_count Number of vertices sets to connect.
   * @param vertex_set Array that says whether some vertex v belongs to
   * some set of terminals or not and if so to which one.
   * @param unpruned_forest_edges Edges of unpruned Steiner Forest.
   * @param forest_edges Output parameter that will store edges of pruned
   * forest.
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
   * @brief Given set of unweighted edges retrives original weight for each edge
   * and store them in output parameter.
   * @param graph Input weighted undirected graph.
   * @param forest_edges Set of edges without weight information.
   * @param steiner_forest_edges Output array used to store result.
   * */
  template <typename G>
  void retrieve_weighted_forest_edges(
    const G& graph,
    std::vector<typename G::edge_t> forest_edges,
    std::vector<typename G::weighted_edge_t>& steiner_forest_edges)
  {
    typedef typename G::edge_iterator_t edge_iterator_t;
    typedef typename G::vertex_t vertex_t;
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
          steiner_forest_edges.push_back(weighted_edge_t(source,
              (*it).target, (*it).weight));
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
   * @brief Removes useless edges from given unpruned Steiner Forest.
   * Returns pruned Steiner Forest.
   */
  template<typename G>
  std::vector<typename G::weighted_edge_t> prune_solution(const G& graph,
      const int vertex_set[],
      std::vector<typename G::weighted_edge_t>& unpruned_forest)
  {
    const size_t vertices_count = graph.get_vertices_count();
    const int max_set_number = *std::max_element(vertex_set,
        vertex_set + vertices_count);
    const int sets_count = max_set_number + 1;
    std::vector<typename G::edge_t> forest_edges;
    prune<G>(vertices_count, sets_count, vertex_set, unpruned_forest,
        forest_edges);
    for (size_t i = 0; i < forest_edges.size(); ++i)
    {
      if (forest_edges[i].source > forest_edges[i].target)
      {
        std::swap(forest_edges[i].source, forest_edges[i].target);
      }
    }
    std::sort(forest_edges.begin(), forest_edges.end());

    std::vector<typename G::weighted_edge_t> pruned_forest_edges;
    retrieve_weighted_forest_edges(graph, forest_edges, pruned_forest_edges);
    return pruned_forest_edges;
  }

  /**
   * @brief Returns pruned minimum spanning tree of given graph.
   * Used as initial solution for local searches.
   */
  template<typename G>
  std::vector<typename G::weighted_edge_t> init_mst(const G& graph,
      const int vertex_set[])
  {
    typedef typename G::edge_iterator_t edge_iterator_t;
    typedef typename G::weighted_edge_t weighted_edge_t;
    const size_t vertices_count = graph.get_vertices_count();
    std::vector<bool> visited(vertices_count, false);
    size_t seed = 0;
    visited[seed] = true;
    std::multiset<weighted_edge_t> pool;
    std::pair<edge_iterator_t, edge_iterator_t> it = graph.out_edges(seed);

    std::vector<typename G::weighted_edge_t> solution;

    while (it.first != it.second)
    {
      pool.insert(weighted_edge_t(seed, (*it.first).target,
          (*it.first).weight));
      it.first++;
    }

    while (!pool.empty())
    {
      auto edge = pool.begin();

      if (!visited[edge->source] || !visited[edge->target])
      {
        size_t new_vertex;
        if (!visited[edge->source])
        {
          new_vertex = edge->source;
        }
        else
        {
          new_vertex = edge->target;
        }

        solution.push_back(*edge);

        visited[new_vertex] = true;

        it = graph.out_edges(new_vertex);

        while (it.first != it.second)
        {
          if (!visited[(*it.first).target])
          {
            pool.insert(weighted_edge_t(new_vertex, (*it.first).target,
                (*it.first).weight));
          }
          it.first++;
        }
      }
      pool.erase(edge);
    }

    solution = prune_solution(graph, vertex_set, solution);
    return solution;
  }
}  //  namespace steiner

#endif  // STEINER_STEINERFORESTUTILS_H_
