// Copyright 2013 <przed_deadlinem_zdarzymy>

#ifndef TSP_CHRISTOFIDES_H_
#define TSP_CHRISTOFIDES_H_

#include<boost/graph/adjacency_list.hpp>
#include<boost/foreach.hpp>

#include<cstdint>
#include<cstddef>
#include<cstdlib>
#include<list>
#include<memory>
#include<iostream>
#include<vector>
#include<string>

#include "blossom5/PerfectMatching.h"
#include "blossom5/GEOM/GeomPerfectMatching.h"


using std::cout;
using std::endl;
using std::list;
using std::unique_ptr;
using std::vector;

using boost::add_edge;
using boost::adjacency_list;
using boost::edge;
using boost::graph_traits;
using boost::num_edges;
using boost::num_vertices;
using boost::out_degree;
using boost::out_edges;
using boost::undirectedS;
using boost::vecS;

namespace tsp {

  /**
   * @brief Creates minimum spanning tree using Prim's algorithm
   * @tparam GraphIn container that store distances between points
   *         that can be accessed by operator (x, y)
   * @tparam GraphOut graph that implements part of boost::graph
   *         interface (add_edge)
   * @param graph container of distances in graph
   * @param out graph to store result tree
   * @param size size of source graph
   */
  template<typename GraphIn, typename GraphOut>
  void mst_prim(const GraphIn &graph, GraphOut &out, size_t size) {
    unique_ptr<int[]> edge(new int[size]);
    unique_ptr<int[]> dist(new int[size]);

    for (size_t i = 0; i < size; ++i) {
      dist[i] = INT_MAX;
    }

    int current = 0;
    int min;
    list<int> unused;
    // We are not adding 0, as it's starting point.
    for (size_t i = 1; i < size; ++i) {
        unused.push_back(i);
    }
    auto next = unused.begin();
    for (size_t i = 1; i < size; ++i) {
      min = INT_MAX;
      for (auto it = unused.begin(); it != unused.end(); ++it) {
          if (dist[*it] > graph(current, *it)) {
            dist[*it] = graph(current, *it);
            edge[*it] = current;
          }
          if (dist[*it] < min) {
            min = dist[*it];
            next = it;
          }
      }
      add_edge(*next, edge[*next], out);
      current = *next;
      unused.erase(next);
    }

#ifdef DEBUG_CHRIST
    typename graph_traits<GraphOut>::edge_iterator edgea, edgeb;
    cout << "Minimum spanning tree:" << endl;
    for (boost::tie(edgea, edgeb) = edges(out); edgea != edgeb; ++edgea) {
      cout << *edgea << endl;
    }
    cout << endl;
#endif
  }


  /**
   * @brief Creates minimum weight perfect matching between odd
   *        vertices to make them even. It uses Blossom V
   *        implementation of Edmond's Blossom algorithm
   *        and it's geometric version when it can.
   * @tparam GraphIn container that store distances between points that
   *         can be accessed by operator (x, y)
   * @tparam GraphOut graph that implements part of boost::graph
   *         interface (out_degree, add_edge, edges)
   * @tparam CoordType used only with geometric version, it's type
   *         of coordinates used in graph
   * @param graph container of distances in graph
   * @param out graph that initially contains some graph and will
   *        contain result graph
   * @param ewt Edge Weigth Type (as described in TSPLIB) supported
   *        types are: CEIL_2D, EUC2_D, ATT
   * @param X array of x-coordinates
   * @param Y array of y-coordinates
   */
  template<typename GraphIn, typename GraphOut, typename CoordType = double>
  void even_odd_vertices(const GraphIn &graph, GraphOut &out,
                         std::string ewt = "", CoordType* X = nullptr,
                         CoordType* Y = nullptr) {
    std::vector<int> oddV;
    for (size_t v = 0; v < graph.size1(); ++v) {
      if (out_degree(v, out) % 2) {
        oddV.push_back(v);
      }
    }

    PerfectMatching::Options options;
    options.verbose = false;
#ifdef DEBUG_CHRIST
    options.verbose = true;
#endif

    if ((ewt == "CEIL_2D" || ewt == "EUC_2D" || ewt == "ATT") &&
          X != nullptr && Y != nullptr) {
        GeomPerfectMatching gpm(oddV.size(), 2);
        gpm.options = options;

        GeomPerfectMatching::GPMOptions gpm_options;
        if (ewt == "CEIL_2D")
          gpm_options.ceil_2d = true;
        else if (ewt == "EUC_2D")
          gpm_options.euc_2d = true;
        else if (ewt == "ATT")
          gpm_options.att_2d = true;
        gpm.gpm_options = gpm_options;

        // Adds points for perfect matching.
        double point[2];
        for (size_t i = 0; i < oddV.size(); ++i) {
          point[0] = X[oddV[i]];
          point[1] = Y[oddV[i]];
          gpm.AddPoint(point);
        }

        // Initial matching
        for (size_t i = 0; i < oddV.size()/2; ++i) {
          gpm.AddInitialEdge(2*i, 2*i + 1);
        }

        gpm.Solve();
        for (size_t i = 0; i < oddV.size(); ++i) {
          int m = oddV[gpm.GetMatch(i)];
          if (m > oddV[i])
            add_edge(oddV[i], m, out);
        }
    } else {
        PerfectMatching pm(oddV.size(), oddV.size() * oddV.size() / 2);
        pm.options = options;
        for (size_t i = 0; i < oddV.size(); ++i) {
          for (size_t j = i + 1; j < oddV.size(); ++j) {
            pm.AddEdge(i, j, graph(oddV[i], oddV[j]));
          }
        }
        pm.Solve();
        for (size_t i = 0; i < oddV.size(); ++i) {
          int m = oddV[pm.GetMatch(i)];
          if (m > oddV[i])
            add_edge(oddV[i], m, out);
        }
    }

#ifdef DEBUG_CHRIST
    typename graph_traits<GraphOut>::edge_iterator edgea, edgeb;
    cout << "Minimum perfect matching of odd vertices" << endl;
    for (boost::tie(edgea, edgeb) = edges(out); edgea != edgeb; ++edgea) {
      cout << *edgea << endl;
    }
    cout << endl;
#endif
  }


  /* @brief ArrayList for efficient finding of eulerian cycle. */
  struct CycleList {
    /* @brief number of vertex in graph */
    int vertex;
    /* @brief next vertex in cycle */
    CycleList* next;
  };


  /**
   * @brief Finds a cycle in given graph that starts from the given vertex
   *        and removes it from graph.
   * @tparam Graph graph that implements part of boost::graph
   *         interface (out_degree, out_edges, target, remove_edge).
   * @param graph source graph that will get modified
   * @param cycle CycleList array to store the found cycle
   * @param added boolean array indicating if vertex was added to toCheck
   * @param toCheck list of vertices that could be not used up
   * @param start number of vertex to start looking for cycle
   */
  template<typename Graph>
  int find_cycle(Graph* graph, CycleList cycle[], bool added[],
                 list<CycleList*> &toCheck, int start) {
#ifdef DEBUG_CHRIST
    cout << "Starting from: " << start << endl;
#endif
    int size = 0;
    int current = start;
    typename graph_traits<Graph>::out_edge_iterator edge;
    while (out_degree(current, *graph)) {
      edge = out_edges(current, *graph).first;
      current = target(*edge, *graph);
      remove_edge(edge, *graph);
      cycle->vertex = current;
      cycle->next = cycle + 1;
      if (!added[current]) {
        added[current] = true;
        toCheck.push_back(cycle);
      }
      cycle = cycle + 1;
      ++size;
    }
#ifdef DEBUG_CHRIST
    cout << "Found cycle size: " << size << endl;
    if (size == 0) {
      exit(0);
    }
#endif
    return size;
  }


  /**
   * @brief Finds eulerian cycle in given graph, saves it to
   *        cycle and deletes all edges from graph.
   * @tparam Graph graph that implements part of boost::graph
   *         interface (num_edges, num_vertices, out_degree)
   * @param graph source graph that will be cleaned from edges
   * @cycle CycleList array to store the found eulerian cycle
   */
  template<typename Graph>
  void find_eulerian_cycle(Graph* graph, CycleList cycle[]) {
    int to_do = num_edges(*graph) + 1;
    unique_ptr<bool[]> added(new bool[num_vertices(*graph)]());
    list<CycleList*> toCheck;
    int done_overall = 1, done_this_iter = 0;
    int start = 0;
    CycleList* ver;
    CycleList* change = cycle;
    change->vertex = start;
    while (to_do != done_overall) {
      done_this_iter = find_cycle<Graph>(graph, cycle + done_overall,
                                         added.get(), toCheck,
                                         change->vertex);
      (cycle + done_overall + done_this_iter - 1)->next = change->next;
      change->next = cycle + done_overall;
      done_overall += done_this_iter;
      while (toCheck.size()) {
        ver = toCheck.front();
        toCheck.pop_front();
        if (out_degree(ver->vertex, *graph)) {
          change = ver;
          break;
        }
      }
    }
#ifdef DEBUG_CHRIST
    cout << "Eulerian cycle" << endl;
    CycleList* current = cycle;
    for (int i = 0; i < to_do; ++i) {
      cout << current->vertex << " ";
      current = current->next;
    }
    cout << endl;
#endif
  }


  /**
   * @brief Creates hamiltonian cycle from given eulerian cycle
   *        by shortcutting.
   * @tparam Cycle cycle with operator[] as access method
   * @param eulerian_cycle source eulerian cycle from which
   *        hamiltonian one is created
   * @param out cycle to store resulting cycle within
   * @param edge_num number of edges in eulerian cycle
   * @param vert_num number of vertices in graph
   */
  template<typename Cycle>
  void find_hamiltonian_cycle(CycleList* eulerian_cycle, Cycle* out,
                              int edge_num, int vert_num) {
    unique_ptr<bool[]> used(new bool[vert_num]());
    CycleList* current = eulerian_cycle;
    int n = 0;
    for (int i = 0; i <= edge_num; ++i) {
      if (!used[current->vertex]) {
        used[current->vertex] = true;
        (*out)[n++] = current->vertex;
      }
      current = current->next;
    }
#ifdef DEBUG_CHRIST
    cout << "Hamiltonian cycle:" << endl;
    for (int i = 0; i < vert_num; ++i) {
      cout << (*out)[i] << " ";
    }
    cout << endl;
#endif
  }


  /**
   * @brief Implementation of Christofides heuristic for
   *        finding 1.5-approximation for instance of
   *        Symmetric Traveling Salesman Problem.
   *        Description of algorithm can be found on Wikipedia.
   * @tparam Graph container that store distances between points
   *         that can be accessed by operator (x, y)
   * @tparam Cycle cycle with operator[] as access method
   * @tparam CoordType used only with geometric version, it's type
   *         of coordinates used in graph
   * @param graph container of distances in graph
   * @param cycle cycle to store resulting cycle within
   * @param size size of problem
   * @param ewt (optional) Edge Weigth Type (as described in TSPLIB)
   *        supported types are: CEIL_2D, EUC2_D, ATT. Used for
   *        geometric version.
   * @param X (optional) array of x-coordinates.
   *        Used for geometric version.
   * @param Y (optional) array of y-coordinates.
   *        Used for geometric version.
   */
  template<typename Graph, typename Cycle, typename CoordType = double>
  void christofides(const Graph &graph, Cycle &cycle, size_t size,
                    std::string ewt = "", CoordType* X = nullptr,
                    CoordType* Y = nullptr) {
    typedef adjacency_list<vecS, vecS, undirectedS> AdjList;
    AdjList _graph(size);
    // 1. Build minimum spanning tree.
    mst_prim<Graph, AdjList>(graph, _graph, size);
    // 2. Create minimum weight perfect mathing over odd vertices in tree.
    even_odd_vertices<Graph, AdjList, CoordType>(graph, _graph, ewt, X, Y);
    int edge_num = num_edges(_graph);
    unique_ptr<CycleList[]> eulerian_cycle(new CycleList[edge_num + 1]);
    // 3. Find eulerian cycle in created multigraph.
    find_eulerian_cycle<AdjList>(&_graph, eulerian_cycle.get());
    // 4. Create hamiltionian cycle from found eulerian cycle.
    find_hamiltonian_cycle<Cycle>(eulerian_cycle.get(), &cycle, edge_num,
                                  size);
  }
}

#endif  // TSP_CHRISTOFIDES_H_
