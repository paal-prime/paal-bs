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

template <typename WALKER>
void runTests(std::string algorithmName,
    const std::vector<std::string>& testNames,
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

    auto HillClimb = SFHillAlgo<graph_t, WALKER>(instance.get_graph(),
        instance.get_vertex_set(),
        initial_solution, iterations);
    dia.test(algorithmName + " HillClimb", HillClimb);

    auto Annealing = SFAnneAlgo <graph_t, WALKER> (
           instance.get_graph(), instance.get_vertex_set(), initial_solution,
           iterations, 1e-9);
    dia.test(algorithmName + " SA 1e-9", Annealing);

    auto Annealing2 = SFAnneAlgo <graph_t, WALKER> (
           instance.get_graph(), instance.get_vertex_set(), initial_solution,
           iterations, 1e-5);
    dia.test(algorithmName + " SA 1e-5", Annealing2);

    auto Annealing3 = SFAnneAlgo <graph_t, WALKER> (
           instance.get_graph(), instance.get_vertex_set(), initial_solution,
           iterations, 1e-2);
    dia.test(algorithmName + " SA 1e-2", Annealing3);

    std::string caption = algorithmName + "-" + testName;
    std::ofstream f(out_dir(caption + ".tex"));
    dia.reduce(.01);
    dia.dump_tex(f, caption, caption);
    std::cout << caption << " done" << std::endl;
  }
}

int main(int argc, char **argv)
{
  Dir out_dir(argc, argv);

  std::vector<std::string> forest_tests =
  {
    "02dEV100K30",
  };

  int iterations = 1000;

  runTests< steiner::ActiveVerticesWalker<graph_t> >("MSTAV", forest_tests,
      FOREST_TESTS_DIR, out_dir, iterations, false);

  runTests< steiner::BreakCycleWalker<graph_t> >("CBC", forest_tests,
      FOREST_TESTS_DIR, out_dir, iterations, false);

  return 0;
}
