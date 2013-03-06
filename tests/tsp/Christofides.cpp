// Copyright 2013 <przed_deadlinem_zdazymy>

#include "tsp/Christofides.h"

#include <gtest/gtest.h>
#include <string>

#include "tsp/TSPLIB.h"
#include "tsp/Cycle.h"
#include "tsp/util.h"

TEST(ChristofidesAlg, ExplicitTest) {
  std::string path = "./TSPLIB/symmetrical";
  const char *filenames[] = { "/pcb442.tsp", "/gr666.tsp",
                              "/att532.tsp", "/brd14051.tsp"};
  tsp::TSPLIB_Directory dir(path);
  tsp::TSPLIB_Matrix mtx;
  tsp::DynCycle cycle;
  for (auto &g : dir.graphs)
    for (int i = 0; i < 3; ++i)
      if (path + filenames[i] == g.second.filename) {
          g.second.load(mtx);
          cycle.resize(mtx.size1());
          tsp::christofides<tsp::TSPLIB_Matrix, tsp::DynCycle>
                                      (mtx, cycle, mtx.size1());
          EXPECT_TRUE(tsp::check_cycle(cycle, mtx.size1()));
          EXPECT_LE(tsp::fitness(mtx, cycle), 1.5 * g.second.optimal_fitness);
        }
}

TEST(ChristofidesAlg, GeoTest) {
  std::string path = "./TSPLIB/symmetrical";
  const char *filenames[] = { "/a280.tsp", "/att532.tsp"};
  tsp::TSPLIB_Directory dir(path);
  tsp::TSPLIB_Matrix mtx;
  tsp::DynCycle cycle, cycle_geo;
  for (auto &g : dir.graphs)
    for (int i = 0; i < 2; ++i)
    if (path + filenames[i] == g.second.filename) {
      std::string ewt = g.second.load(mtx);
      cycle.resize(mtx.size1());
      tsp::christofides<tsp::TSPLIB_Matrix, tsp::DynCycle>
                                  (mtx, cycle, mtx.size1());
      cycle_geo.resize(mtx.size1());
      tsp::christofides<tsp::TSPLIB_Matrix, tsp::DynCycle>
        (mtx, cycle_geo, mtx.size1(), ewt, mtx.X.get(), mtx.Y.get());
      EXPECT_TRUE(tsp::check_cycle(cycle_geo, mtx.size1()));
      EXPECT_LE(tsp::fitness(mtx, cycle_geo), 1.5 * g.second.optimal_fitness);
      EXPECT_GE(1.1 * tsp::fitness(mtx, cycle), tsp::fitness(mtx, cycle_geo));
    }
}
