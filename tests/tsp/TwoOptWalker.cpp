#include <gtest/gtest.h>

#include <vector>
#include <boost/numeric/ublas/matrix.hpp>
#include <random>

#include "tsp/TwoOptWalker.h"
#include "tsp/util.h"

typedef boost::numeric::ublas::matrix<int> Matrix;

void set_matrix(Matrix &matrix)
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
  matrix.resize(n, n);
  for (size_t i = 0; i < n; ++i)
    for (size_t j = i; j < n; ++j)
      matrix(i, j) = matrix(j, i) = A[j][i];
}

TEST(tsp_TwoOptWalker, constructor)
{
  Matrix m;
  std::vector<size_t> c = { 0, 4, 2, 1, 3 };
  set_matrix(m);
  tsp::TwoOptWalker<Matrix> walker(m, c);
  EXPECT_EQ(tsp::fitness(m, c), walker.current_fitness());
}

TEST(tsp_TwoOptWalker, CRASH_TEST)
{
  Matrix m;
  std::vector<size_t> c = { 0, 4, 2, 1, 3 };
  std::mt19937 random(7628342);
  set_matrix(m);
  tsp::TwoOptWalker<Matrix> walker(m, c);
  for (size_t i = 20; i--;)
  {
    walker.prepare_step(i * .05, random);
    walker.current_fitness();
    walker.next_fitness();
    if (i & 1) walker.make_step();
  }
}

