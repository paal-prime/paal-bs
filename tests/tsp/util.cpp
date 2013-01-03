#include <boost/numeric/ublas/matrix.hpp>
#include <gtest/gtest.h>

#include <random>
#include <vector>

#include "tsp/util.h"

TEST(tsp_util, fitness)
{
  enum { n = 5 };
  static const int A[n][n] =
  {
    {0},
    {1, 0},
    {8, 6, 0},
    {2, 5, 3, 0},
    {1, 4, 2, 7, 0}
  };
  boost::numeric::ublas::matrix<int> matrix(5, 5);
  for (size_t i = 0; i < n; ++i)
    for (size_t j = i; j < n; ++j)
      matrix(i, j) = matrix(j, i) = A[j][i];
  std::vector<int> cycle = {4, 2, 0, 3, 1};
  EXPECT_EQ(2 + 8 + 2 + 5 + 4, tsp::fitness(matrix, cycle));
}

TEST(tsp_util, cycle_shuffle)
{
  enum { n = 10 };
  std::vector<int> cycle;
  std::mt19937 random(726384);
  tsp::cycle_shuffle(cycle, n, random);
  EXPECT_EQ(n, cycle.size());
  int C[n] = {};
for (int i : cycle) C[i]++;
for (int c : C) EXPECT_EQ(1, c);
}

TEST(tsp_util, cycle_reverse)
{
  std::vector<int> cycle = { 6, 3, 4, 1, 5, 2, 0 };
  std::vector<int> cycle_after = { 5, 1, 4, 3, 6, 2, 0 };
  tsp::cycle_reverse(cycle, 0, 5);
  EXPECT_EQ(cycle_after, cycle);
}

TEST(tsp_util, Split)
{
  enum { n = 10 };
  tsp::Split split;
  std::mt19937 random(289374);
  for (size_t i = 0; i < 10; ++i)
  {
    split.generate(n, random);
    EXPECT_LT(split.begin, split.end);
    EXPECT_LE(split.end, n);
  }
}

