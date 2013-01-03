#include <gtest/gtest.h>

#include <string>

#include "tsp/TSPLIB.h"

TEST(TSPLIB_Matrix, constructor)
{
  tsp::TSPLIB_Matrix m;
  EXPECT_EQ(m.size1(), 0);
  EXPECT_TRUE(m.pos.empty());
  EXPECT_EQ(m.mtx.size1(), 0);
}

TEST(TSPLIB_Matrix, resize)
{
  tsp::TSPLIB_Matrix m;
  m.resize(10, 10);
  EXPECT_EQ(m.size1(), 10);
  EXPECT_TRUE(m.pos.empty());
  EXPECT_EQ(m.mtx.size1(), 10);
  m.resize(20, 20, m.eucl_dist);
  EXPECT_EQ(m.size1(), 20);
  EXPECT_EQ(m.pos.size(), 20);
  EXPECT_EQ(m.mtx.size1(), 0);
}

TEST(TSPLIB_Directory, dist)
{
  // canonical cycle lengths for symmetrical tsp problems' sample
  // http://www.iwr.uni-heidelberg.de/groups/comopt/software/TSPLIB95/DOC.PS
  std::string path = "./TSPLIB/symmetrical";
  const char *filenames[] = { "/pcb442.tsp", "/gr666.tsp", "/att532.tsp" };
  int fitnesses[] = { 221440, 423710, 309636 };
  tsp::TSPLIB_Directory dir(path);
  tsp::TSPLIB_Matrix mtx;
for (auto &g : dir.graphs)
    for (int i = 0; i < 3; ++i)
      if (path + filenames[i] == g.filename)
      {
        g.load(mtx);
        int fitness = 0;
        for (size_t j = 0; j < mtx.size1(); ++j)
          fitness += mtx(j, (j + 1) % mtx.size1());
        EXPECT_EQ(fitnesses[i], fitness);
      }
}

