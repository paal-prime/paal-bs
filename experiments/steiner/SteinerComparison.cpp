#include <algorithm>
#include <string>
#include <vector>
#include <iostream>  // NOLINT(readability/streams)

#include <paal/GridTable.h>
#include <result_dir.h>
#include <format.h>

#include "experiments/steiner/Algo.h"

#define TREE_TESTS_DIR "./tests/steiner/paal_sf_tests/tree_tests/"
#define FOREST_TESTS_DIR "./tests/steiner/paal_sf_tests/forest_tests/"

typedef graph::AdjacencyMatrix<graph::undirected, double> graph_t;

void runTests(std::string outputName, Dir& resdir, std::vector<std::string> testsNames, std::string TESTS_DIR,
  int CBCIterations, int MSTAVIterations, bool loadOpts)
{
  paal::GridTable table;
  if(loadOpts)
  {
    table.push_algo("Optimum");
  }
  table.push_algo("2-Approximation");
  table.push_algo("MSTAV HillClimb");
  table.push_algo("CBC HillClimb");

  for (auto testName: testsNames)
  {
    steiner::SteinerForestInstance<graph_t> instance;

    std::string inputFilepath = TESTS_DIR + testName + ".in";
    std::string optimumFilepath;
    if(loadOpts)
    {
      optimumFilepath = TESTS_DIR + testName + ".out";
    }
    instance.load(inputFilepath, optimumFilepath);

    auto initial_solution = steiner::init_mst(instance.get_graph(), instance.get_vertex_set());

    table.columns.push_back((format("%", testName)));

    if(loadOpts)
    {
      table.records[0].results.push_back(instance.get_best_known_cost());
    }

    auto approximation = SFApxAlgo<graph_t>(instance.get_graph(), instance.get_vertex_set());
    table.records[0 + loadOpts].test(approximation);

    auto MSTAVHillClimb = SFHillAlgo<graph_t, steiner::ActiveVerticesWalker<graph_t> >(instance.get_graph(), instance.get_vertex_set(), initial_solution, MSTAVIterations);
    table.records[1 + loadOpts].test(MSTAVHillClimb);

    auto CBCHillClimb = SFHillAlgo<graph_t, steiner::BreakCycleWalker<graph_t> >(instance.get_graph(), instance.get_vertex_set(), initial_solution, CBCIterations);
    table.records[2 + loadOpts].test(CBCHillClimb);

    std::cerr << testName + " comparison done" << std::endl;
  }

  std::ofstream tex(resdir(outputName + ".tex"));
  table.dump_tex(tex);
}

int main(int argc, char **argv)
{
  Dir resdir(argc, argv);

  std::vector<std::string> tree_smaller_tests = {
    "alut2764", "b13", "berlin52", "diw0250", "msm3277", "d15"
  };

  std::vector<std::string> forest_smaller_tests = {
    "01dEV100K20", "01sEV1000K150", "01sEV100K20", "01dRV100K20",
  };

  std::vector<std::string> tree_larger_tests = {
    "alue2087", "d11", "diw0445", "e02", "taq0431", "msm3676",
  };

  std::vector<std::string> forest_larger_tests = {
    "04sEV1000K250", "03sEV1000K150", "01sRV1000K150", "01sRV1000K250",
  };

  runTests("comparisonSmallerTreeTests", resdir, tree_smaller_tests, TREE_TESTS_DIR, 1000, 1000, true);
  runTests("comparisonSmallerForestTests", resdir, forest_smaller_tests, FOREST_TESTS_DIR, 1000, 1000, false);

  runTests("comparisonLargerTreeTests", resdir, tree_larger_tests, TREE_TESTS_DIR, 1000, 1000, true);
  runTests("comparisonLargerForestTests", resdir, forest_larger_tests, FOREST_TESTS_DIR, 1000, 1000, false);

  return 0;
}
