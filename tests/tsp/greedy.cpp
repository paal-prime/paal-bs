#include <gtest/gtest.h>


#include <boost/numeric/ublas/matrix.hpp>

#include "tsp/greedy.h"

typedef boost::numeric::ublas::matrix<int> Matrix;

inline void set_matrix(Matrix &matrix)
{
  enum { n = 5 };
  static const int A[n][n] =
  {
    {0},
    {1, 0},
    {8, 6, 0},
    {2, 5, 3, 0},
    {5, 4, 2, 7, 0}
  };
  matrix.resize(n, n);
  for (size_t i = 0; i < n; ++i)
    for (size_t j = i; j < n; ++j)
      matrix(i, j) = matrix(j, i) = A[j][i];
}

TEST(tsp_greedy,sanity)
{
  Matrix m;
  set_matrix(m);
  std::vector<size_t> cycle;
  tsp::greedy(m,cycle);
  std::vector<size_t> expected = { 0, 1, 4, 2, 3 };
  EXPECT_EQ(expected,cycle);
}
