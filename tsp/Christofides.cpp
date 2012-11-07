#include <cassert>
#include <sys/time.h>
#include <string>

#include "Christofides.h"
#include "Cycle.h"
#include "Matrix.h"
#include "util.h"

#include "TSPLIB.h"

using std::cout;
using std::endl;
using std::string;
using namespace tsp;

static std::string path = "/home/jachex/MIMUW/5semestr/ZPP/paal/TSPLIB/symmetrical";
static const double NANO = 1000000;

typedef TSPLIB_Directory::Graph TSPLIB_Graph;
typedef TSPLIB_Matrix Matrix;
typedef DynCycle Cycle;

void run(const TSPLIB_Graph &tsplib_graph) {
	timespec start, end;
	Cycle cycle;
	Matrix graph;
	string ewt;
	ewt = tsplib_graph.load(graph);
	cycle.resize(graph.size1());
	std::cout << "Size: " << graph.size1() << std::endl;
	clock_gettime(CLOCK_REALTIME, &start);
	cout << ewt << endl;
	if(ewt == "EUC_2D" || ewt == "CEIL_2D") {
		christofides<Matrix, Cycle, double>(graph, cycle, graph.size1(), ewt, graph.X.get(), graph.Y.get());
	} else {
		christofides<Matrix, Cycle>(graph, cycle, graph.size1());
	}
	clock_gettime(CLOCK_REALTIME, &end);

	Matrix::value_type len = fitness(graph, cycle);
	std::cout << tsplib_graph.filename << " "
			  << tsplib_graph.optimal_fitness << " "
			  << len << " "
			  << len/tsplib_graph.optimal_fitness << " "
			  << (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec)/1000000 << "ms "
			  << std::endl;
	assert(check_cycle(cycle, graph.size1()));
}

int main(int argc, char* argv[]) {
	TSPLIB_Directory tsplib_dir(path);

	if (argc == 1) {
		for (auto &tsplib_graph : tsplib_dir.graphs) {
			run(tsplib_graph.second);
		}
	} else {
		for (int i = 1; i < argc; ++i) {
			run(tsplib_dir.graphs[argv[i]]);
		}
	}
	return 0;
}
