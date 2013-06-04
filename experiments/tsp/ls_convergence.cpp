#include <vector>
#include <iostream>
#include <fstream>

#include "tsp/TSPLIB.h"
#include "paal/FIDiagram.h"

#include "format.h"
#include "result_dir.h"

#include "experiments/tsp/Algo.h"

int main(int argc, char **argv)
{
  Dir out_dir(argc, argv);
  tsp::TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  std::vector<std::string> graph_ids =
  {
    "brazil58",
    "d15112",
    "ulysses22",
  };
  std::vector<size_t> it =
  {
    100000,
    1000000,
    10000,
  };

  Matrix matrix;
  for (size_t i = 0; i < graph_ids.size(); ++i)
  {
    paal::FIDiagram dia(0);
    dir.graphs[graph_ids[i]].load(matrix);
    dia.test("hill climb", HillAlgo(matrix, it[i]));
    dia.test("annealing 1e-9", AnneAlgo(matrix, it[i], 1e-9));
    dia.test("annealing 1e-5", AnneAlgo(matrix, it[i], 1e-5));
    dia.test("annealing 1e-2", AnneAlgo(matrix, it[i], 1e-2));
    std::ofstream f(out_dir(graph_ids[i] + ".tex"));
    dia.reduce(.01);
    dia.dump_tex(f, graph_ids[i]);
    std::cout << graph_ids[i] << " done" << std::endl;
  }
  return 0;
}
