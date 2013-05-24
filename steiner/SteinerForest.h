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
#include "steiner/SteinerForestUtils.h"

namespace steiner
{
  /**
   * @brief Finds unpruned Steiner Forest (forest that might contain useless
   * edges that will be removed (pruned) later.
   * @param graph Weighted undirected graph.
   * @param vertex_set Array that says whether given vertex v belongs to some
   * set of terminals or not and if so to which one.
   * @param unpruned_forest_edges Output parameter that will store edges of
   * found unpruned forest.
   * */
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
    memset(set_cardinality_counter, 0,
           sizeof(set_cardinality[0]) * vertices_count);
    std::vector<boost::heap::skew_heap<int>> compound_terminals(vertices_count);
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
   * @brief Finds Steiner Forest for given weighted undirected graph and set of
   * terminals.
   * @param graph Weighted undirected graph,
   * @param vertex_set It's an array that says whether vertex v is a terminal and
   * if so to which set it belongs. If vertex_set[v] == -1 then v is not a
   * terminal otherwise vertex_set[v] is corresponding set's number.
   * @param steiner_forest_edges array used to store edges of found
   * Steiner Forest.
   **/
  template <typename G>
  void steiner_forest(
    const G& graph,
    const int vertex_set[],
    std::vector<typename G::weighted_edge_t>& steiner_forest_edges)
  {
    typedef typename G::weighted_edge_t weighted_edge_t;

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

    steiner_forest_edges = prune_solution<G>(graph, vertex_set,
        unpruned_forest_edges);
  }

}  //  namespace steiner

#endif  // STEINER_STEINERFOREST_H_
