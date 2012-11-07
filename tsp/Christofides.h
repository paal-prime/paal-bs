// Piotr Jaszkowski
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
  // Creates minimum spanning tree using Prim's algorithm.
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


  // Creates minimum weight perfect matching between odd vertices
  // to make them even. It uses Blossom V implementation of Edmond's
  // Blossom algorithm.
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
	GeomPerfectMatching::GPMOptions gpm_options;
	options.verbose = false;
#ifdef DEBUG_CHRIST
    options.verbose = true;
#endif

    if (ewt != "" && X != nullptr && Y != nullptr) {
    	GeomPerfectMatching gpm(oddV.size(), 2);
    	gpm.options = options;
    	if(ewt == "CEIL_2D")
    		gpm_options.euc_2d = false;
    	gpm.gpm_options = gpm_options;
    	double point[2];
    	for (size_t i = 0; i < oddV.size(); ++i) {
    		point[0] = X[oddV[i]];
    		point[1] = Y[oddV[i]];
    		gpm.AddPoint(point);
    	}
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


  struct CycleList {
    int vertex;
    CycleList* next;
  };


  // Finds a cycle in given graph that starts from the given vertex
  // and removes it from graph.
  // Graph should be implementing BGL interface.
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


  // Finds eulerian cycle in given graph, saves it to cycle
  // and deletes all edges from graph.
  template<typename Graph>
  void find_eulerian_cycle(Graph* graph, CycleList cycle[]) {
    int to_do = num_edges(*graph) + 1;
    unique_ptr<bool[]> added(new bool[num_vertices(*graph)]());
    list<CycleList*> toCheck;
    int done_overall = 1, done_this_iter = 0;
    // TODO(pjaszkowski): Make it random?
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
    CycleList* current = cycle.get();
    for (int i = 0; i < to_do; ++i) {
      cout << current->vertex << " ";
      current = current->next;
    }
    cout << endl;
#endif
  }


  // Given eulerian cycle creates hamiltonian cycle.
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


  template<typename Graph, typename Cycle, typename CoordType = double>
  void christofides(const Graph &graph, Cycle &cycle, size_t size,
		  	  	    std::string ewt = "", CoordType* X = nullptr,
		  	  	    CoordType* Y = nullptr) {
    typedef adjacency_list<vecS, vecS, undirectedS> AdjList;
    AdjList _graph(size);
    mst_prim<Graph, AdjList>(graph, _graph, size);

    if (X != nullptr && Y != nullptr && ewt != "") {
    	even_odd_vertices<Graph, AdjList, CoordType>(graph, _graph, ewt, X, Y);
    } else {
    	even_odd_vertices<Graph, AdjList>(graph, _graph);
    }

    int edge_num = num_edges(_graph);
    unique_ptr<CycleList[]> eulerian_cycle(new CycleList[edge_num + 1]);
    find_eulerian_cycle<AdjList>(&_graph, eulerian_cycle.get());
    find_hamiltonian_cycle<Cycle>(eulerian_cycle.get(), &cycle, edge_num,
                                  size);
  }
}

#endif  // TSP_CHRISTOFIDES_H_
