#include <vector>
#include <iostream> // NOLINT
#include <fstream> // NOLINT
#include <string>
#include <cstdlib>

#include "paal/FIDiagram.h"

#include "format.h"
#include "result_dir.h"

#include "experiments/steiner/Algo.h"
#include "steiner/SteinerForestInstance.h"

#define TREE_TESTS_DIR "./tests/steiner/paal_sf_tests/tree_tests/"
#define FOREST_TESTS_DIR "./tests/steiner/paal_sf_tests/forest_tests/"

typedef graph::AdjacencyMatrix<graph::undirected, double> graph_t;

void runTests(const std::vector<std::string>& testNames,
    std::string TESTS_DIR, Dir& out_dir, int iterations,
    bool loadOpts = false)
{
for (auto testName : testNames)
  {
    std::string inputFilepath = TESTS_DIR + testName + ".in";
    std::string optimumFilepath;
    if (loadOpts)
    {
      optimumFilepath = TESTS_DIR + testName + ".out";
    }

    steiner::SteinerForestInstance<graph_t> instance;
    instance.load(inputFilepath, optimumFilepath);

    paal::FIDiagram dia(loadOpts ? instance.get_best_known_cost() : 0);

    auto initial_solution = steiner::init_mst(instance.get_graph(),
        instance.get_vertex_set());

    auto MSTAVHillClimb =
      SFHillAlgo<graph_t, steiner::ActiveVerticesWalker<graph_t> >
      (instance.get_graph(), instance.get_vertex_set(),
       initial_solution, iterations);
    dia.test("MSTAV HillClimb", MSTAVHillClimb);

    auto CBCHillClimb = SFHillAlgo<graph_t, steiner::BreakCycleWalker<graph_t> >
        (instance.get_graph(), instance.get_vertex_set(),
         initial_solution, iterations);
    dia.test("CBC HillClimb", CBCHillClimb);

    std::string caption = "CBCvsMSTAV" + std::string("-") + testName;
    std::ofstream
    f(out_dir(caption + ".tex"));
    dia.reduce(.01);
    dia.dump_tex(f, caption, caption);
    std::cout << caption << " done";
    std::cout << std::endl;
  }
}

int main(int argc, char **argv)
{
  Dir out_dir(argc, argv);

  std::vector<std::string> tree_tests =
  {
    "d05", "gap1307",
  };

  int iterations = 1000;

  runTests(tree_tests, TREE_TESTS_DIR, out_dir, iterations, true);

  return 0;
}
