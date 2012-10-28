//Piotr Jaszkowski
#ifndef tsp_Christofides_h
#define tsp_Christofides_h

#include<boost/graph/adjacency_list.hpp>
#include<boost/foreach.hpp>
#include<cstdint>
#include<cstddef>
#include<cstdlib>
#include<memory>
#include<iostream>
#include<vector>

#include "blossom5/PerfectMatching.h"

using namespace std;
using namespace boost;

namespace tsp
{
	// Creates minimum spanning tree using Prim's algorithm.
	template<typename GraphIn, typename GraphOut>
		void mst_prim(const GraphIn &graph, GraphOut &out, size_t size)
	{
		unique_ptr<bool[]> used(new bool[size]());
		unique_ptr<int[]> edge(new int[size]());

		unique_ptr<int[]> dist(new int[size]);
		for(size_t i = 0; i < size; ++i) {
			dist[i] = INT_MAX;
		}

		int current = 0, next;
		int min;
		for(size_t i = 1; i < size; ++i) {
			used[current] = true;
			next = current;
			min = INT_MAX;
			// TODO(pjaszkowski): use list of unused vectors.
			for(size_t j = 0; j < size; ++j) {
				if(!used[j]) {
					if(dist[j] > graph(current, j)) {
						dist[j] = graph(current, j);
						edge[j] = current;
					}
					if(dist[j] < min) {
						min = dist[j];
						next = j;
					}
				}
			}
			add_edge(next, edge[next], out);
			current = next;
		}

		typename graph_traits<GraphOut>::edge_iterator edgea, edgeb;
		cout << "Minimum spanning tree:" << endl;
		for(boost::tie(edgea, edgeb) = edges(out); edgea != edgeb; ++edgea) {
			cout << *edgea << endl;
		}
		cout << endl;
	}

	// Creates minimum weight perfect matching between odd vertices
	// to make them even. It uses Blossom V implementation of Edmond's
	// Blossom algorithm.
	template<typename GraphIn, typename GraphOut>
		void even_odd_vertices(const GraphIn &graph, GraphOut &out)
	{
		vector<int> oddV;
		BOOST_FOREACH(int v, vertices(out)) {
			if(out_degree(v, out) % 2) {
				oddV.push_back(v);
			}
		}
		PerfectMatching pm(oddV.size(), oddV.size() * oddV.size() / 2);
		for(size_t i = 0; i < oddV.size(); ++i) {
			for(size_t j = i + 1; j < oddV.size(); ++j) {
				pm.AddEdge(i, j, graph(oddV[i], oddV[j]));
			}
		}
		pm.Solve();
		for(size_t i = 0; i < oddV.size(); ++i) {
			int m = oddV[pm.GetMatch(i)];
			if(m > oddV[i])
				add_edge(oddV[i], oddV[pm.GetMatch(i)], out);
		}

		typename graph_traits<GraphOut>::edge_iterator edgea, edgeb;
		cout << "Minimum perfect matching of odd vertices" << endl;
		for(boost::tie(edgea, edgeb) = edges(out); edgea != edgeb; ++edgea) {
			cout << *edgea << endl;
		}
		cout << endl;
	}

	struct CycleList {
		int vertex;
		CycleList* next;
	};

	// Finds a cycle in given graph that starts from given vertex
	// and removes it from graph.
	// Graph should be implementing BGL interface.
	template<typename Graph>
		int find_cycle(Graph &graph, CycleList cycle[], bool added[],
					   list<CycleList*> &toCheck, int start) {
			cout << "Starting from: " << start << endl;
			int size = 0;
			int current = start;
			typename graph_traits<Graph>::out_edge_iterator edge;
			while (out_degree(current, graph)) {
				if(!added[current]) {
					added[current] = true;
					toCheck.push_back(cycle);
				}
				edge = out_edges(current, graph).first;
				current = target(*edge, graph);
				remove_edge(edge, graph);
				cycle->vertex = current;
				cycle->next = cycle + 1;
				cycle = cycle + 1;
				++size;
			}
			cout << "Found cycle size: " << size << endl;
			return size;
	}

	// Finds eulerian cycle in given graph, saves it to cycle
	// and deleted all edges from graph.
	template<typename Graph>
		unique_ptr<CycleList[]> find_eulerian_cycle(Graph &graph) {
			int to_do = num_edges(graph) + 1;
			unique_ptr<CycleList[]> cycle(new CycleList[to_do]);
			unique_ptr<bool[]> added(new bool[num_vertices(graph)]());
			std::list<CycleList*> toCheck;
			int done = 1, done2 = 0;
			// TODO: Make it random?
			int start = 0;
			CycleList* ver;
			CycleList* change = cycle.get();
			change->vertex = start;
			while(to_do != done) {
				done2 = find_cycle<Graph>(graph, cycle.get() + done,
										  added.get(), toCheck,
										  change->vertex);
				(cycle.get() + done + done2 - 1)->next = change->next;
				change->next = cycle.get() + done;
				done += done2;
				while(toCheck.size()) {
					ver = toCheck.front();
					toCheck.pop_front();
					if(out_degree(ver->vertex, graph)) {
						change = ver;
						break;
					}
				}
			}
			cout << "Eulerian cycle" << endl;
			CycleList* current = cycle.get();
			for(int i = 0; i < to_do; ++i) {
				cout << current->vertex << " ";
				current = current->next;
			}
			cout << endl;
			return std::move(cycle);
	}

	// Given eulerian cycle creates hamiltonian cycle.
	template<typename Cycle>
		void find_hamiltonian_cycle(CycleList* eulerian_cycle, Cycle &out,
									int edge_num, int vert_num) {
			std::unique_ptr<bool[]> used(new bool[vert_num]());
			CycleList* current = eulerian_cycle;
			int n = 0;
			for(int i = 0; i <= edge_num; ++i) {
				if(!used[current->vertex]) {
					used[current->vertex] = true;
					out[n++] = current->vertex;
				}
				current = current->next;
			}
			cout << "Hamiltonian cycle:" << endl;
			for(int i = 0; i < vert_num; ++i) {
				cout << out[i] << " ";
			}
			cout << endl;
	}

	template<typename Graph, typename Cycle>
		void christofides(const Graph &graph, Cycle &cycle, size_t size)
	{
		typedef adjacency_list<vecS, vecS, undirectedS> AdjList;
		AdjList _graph(size);
		mst_prim<Graph, AdjList>(graph, _graph, size);
		even_odd_vertices<Graph, AdjList>(graph, _graph);
		int edge_num = num_edges(_graph);
		unique_ptr<CycleList[]> eulerian_cycle;
		eulerian_cycle = find_eulerian_cycle<AdjList>(_graph);

		unique_ptr<int[]> hamiltonian_cycle;
		find_hamiltonian_cycle<Cycle>(eulerian_cycle.get(), cycle, edge_num,
									  size);
	}
}

#endif
