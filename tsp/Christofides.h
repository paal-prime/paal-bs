// Copyright 2013 <przed_deadlinem_zdazymy>

#ifndef TSP_CHRISTOFIDES_H_
#define TSP_CHRISTOFIDES_H_

#include<boost/graph/adjacency_list.hpp>

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
#include "tsp/util.h"

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
    std::vector<int> edge(size);
    std::vector<int> dist(size, INT_MAX);

    int current = 0;
    int min;
    std::list<int> unused;
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
      boost::add_edge(*next, edge[*next], out);
      current = *next;
      unused.erase(next);
    }

#ifdef DEBUG_CHRIST
    typename boost::graph_traits<GraphOut>::edge_iterator edgea, edgeb;
    std::cout << "Minimum spanning tree:" << std::endl;
    for (boost::tie(edgea, edgeb) = boost::edges(out); edgea != edgeb;
         ++edgea) {
      std::cout << *edgea << std::endl;
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
   * @tparam Points used only with geometric version, it's type
   *         of points container
   * @param graph container of distances in graph
   * @param out graph that initially contains some graph and will
   *        contain result graph
   * @param ewt Edge Weigth Type (as described in TSPLIB) supported
   *        types are: CEIL_2D, EUC2_D, ATT
   * @param points is an pointer to instance of Points that
   *        holds graph points coordinates.
   */
  template<typename GraphIn, typename GraphOut, typename Points = std::vector<Point> >
  void even_odd_vertices(const GraphIn &graph, GraphOut &out,
                         std::string ewt = "", Points* points = nullptr) {
    std::vector<int> oddV;
    for (size_t v = 0; v < graph.size1(); ++v) {
      if (boost::out_degree(v, out) % 2) {
        oddV.push_back(v);
      }
    }

    PerfectMatching::Options options;
    options.verbose = false;
#ifdef DEBUG_CHRIST
    options.verbose = true;
#endif

    if ((ewt == "CEIL_2D" || ewt == "EUC_2D" || ewt == "ATT") &&
          points != nullptr) {
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
          point[0] = (*points)[oddV[i]].x;
          point[1] = (*points)[oddV[i]].y;
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
            boost::add_edge(oddV[i], m, out);
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
            boost::add_edge(oddV[i], m, out);
        }
    }

#ifdef DEBUG_CHRIST
    typename boost::graph_traits<GraphOut>::edge_iterator edgea, edgeb;
    std::cout << "Minimum perfect matching of odd vertices" << std::endl;
    for (boost::tie(edgea, edgeb) = boost::edges(out); edgea != edgeb;
         ++edgea) {
      std:cout << *edgea << std::endl;
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
  int find_cycle(Graph* graph, CycleList cycle[], std::vector<bool> &added,
                 std::list<CycleList*> &toCheck, int start) {
#ifdef DEBUG_CHRIST
    cout << "Starting from: " << start << endl;
#endif
    int size = 0;
    int current = start;
    typename boost::graph_traits<Graph>::out_edge_iterator edge;
    while (out_degree(current, *graph)) {
      edge = boost::out_edges(current, *graph).first;
      current = boost::target(*edge, *graph);
      boost::remove_edge(edge, *graph);
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
    std::cout << "Found cycle size: " << size << std::endl;
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
   * @param cycle CycleList array to store the found eulerian cycle
   */
  template<typename Graph>
  void find_eulerian_cycle(Graph* graph, CycleList cycle[]) {
    int to_do = boost::num_edges(*graph) + 1;
    std::vector<bool> added(boost::num_vertices(*graph), false);
    std::list<CycleList*> toCheck;
    int done_overall = 1, done_this_iter = 0;
    int start = 0;
    CycleList* ver;
    CycleList* change = cycle;
    change->vertex = start;
    while (to_do != done_overall) {
      done_this_iter = find_cycle<Graph>(graph, cycle + done_overall,
                                         added, toCheck,
                                         change->vertex);
      (cycle + done_overall + done_this_iter - 1)->next = change->next;
      change->next = cycle + done_overall;
      done_overall += done_this_iter;
      while (toCheck.size()) {
        ver = toCheck.front();
        toCheck.pop_front();
        if (boost::out_degree(ver->vertex, *graph)) {
          change = ver;
          break;
        }
      }
    }
#ifdef DEBUG_CHRIST
    std::cout << "Eulerian cycle" << std::endl;
    CycleList* current = cycle;
    for (int i = 0; i < to_do; ++i) {
      std::cout << current->vertex << " ";
      current = current->next;
    }
    std::cout << std::endl;
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
  void find_hamiltonian_cycle(CycleList eulerian_cycle[], Cycle* out,
                              int edge_num, int vert_num) {
    std::vector<bool> used(vert_num, false);
    CycleList* current = &eulerian_cycle[0];
    int n = 0;
    for (int i = 0; i <= edge_num; ++i) {
      if (!used[current->vertex]) {
        used[current->vertex] = true;
        (*out)[n++] = current->vertex;
      }
      current = current->next;
    }
#ifdef DEBUG_CHRIST
    std::cout << "Hamiltonian cycle:" << std::endl;
    for (int i = 0; i < vert_num; ++i) {
      std::cout << (*out)[i] << " ";
    }
    std::cout << std::endl;
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
   * @tparam Points used only with geometric version, it's type
   *         of points container
   * @param graph container of distances in graph
   * @param cycle cycle to store resulting cycle within
   * @param size size of problem
   * @param ewt (optional) Edge Weigth Type (as described in TSPLIB)
   *        supported types are: CEIL_2D, EUC2_D, ATT. Used for
   *        geometric version.
   * @param points is an pointer to instance of Points that
   *        holds graph points coordinates.
   */
  template<typename Graph, typename Cycle, typename Points = std::vector<Point> >
  void christofides(const Graph &graph, Cycle &cycle, size_t size,
                    std::string ewt = "", Points* points = nullptr) {
    typedef boost::adjacency_list<boost::vecS, boost::vecS,
        boost::undirectedS> AdjList;
    AdjList _graph(size);
    // 1. Build minimum spanning tree.
    mst_prim<Graph, AdjList>(graph, _graph, size);
    // 2. Create minimum weight perfect mathing over odd vertices in tree.
    even_odd_vertices<Graph, AdjList, Points>(graph, _graph, ewt, points);
    int edge_num = num_edges(_graph);
    std::unique_ptr<CycleList[]> eulerian_cycle(new CycleList[edge_num + 1]);
    // 3. Find eulerian cycle in created multigraph.
    find_eulerian_cycle<AdjList>(&_graph, eulerian_cycle.get());
    // 4. Create hamiltionian cycle from found eulerian cycle.
    find_hamiltonian_cycle<Cycle>(eulerian_cycle.get(), &cycle, edge_num,
                                  size);
  }
}

#endif  // TSP_CHRISTOFIDES_H_
