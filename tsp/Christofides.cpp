#include "Christofides.h"
#include "Cycle.h"
#include "Matrix.h"

using namespace tsp;

static const size_t SIZE = 7;
typedef StaMatrix<int, SIZE> Matrix;
typedef StaCycle<SIZE> Cycle;

int main() {
	Matrix graph;
	for(size_t i = 0; i < SIZE; ++i) {
		for(size_t j = 0; j < SIZE; ++j) {
			graph(i, j) = 1000;
		}
	}
	graph(0, 1) = graph(1, 0) = 7;
	graph(0, 3) = graph(3, 0) = 5;
	graph(1, 2) = graph(2, 1) = 8;
	graph(1, 3) = graph(3, 1) = 9;
	graph(1, 4) = graph(4, 1) = 7;
	graph(2, 4) = graph(4, 2) = 5;
	graph(3, 4) = graph(4, 3) = 15;
	graph(3, 5) = graph(5, 3) = 6;
	graph(4, 5) = graph(5, 4) = 8;
	graph(4, 6) = graph(6, 4) = 9;
	graph(5, 6) = graph(6, 5) = 11;
	Cycle cycle;
	christofides<Matrix, Cycle>(graph, cycle, SIZE);
	return 0;
}
