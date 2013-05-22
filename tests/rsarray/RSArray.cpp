#include <gtest/gtest.h>

#include <vector>

#include "rsarray/RSArray.h"

typedef rsarray::RSArray<int> RSA;

template<typename Cycle> void identity(Cycle &cycle, size_t n)
{
  cycle.resize(n);
  for (size_t i = 0; i < n; ++i) cycle[i] = i;
}

template<typename Array> std::vector<int> to_vector(const Array &A)
{
  std::vector<int> V(A.size());
  for (size_t i = 0; i < A.size(); ++i) V[i] = A[i];
  return V;
}

template<typename Cycle> void cycle_reverse(Cycle &cycle, size_t l, size_t r)
{
  while (l < r) std::swap(cycle[l++], cycle[--r]);
}

TEST(RSArray, constructor)
{
  RSA cycle;
  EXPECT_FALSE(cycle.size());
}

TEST(RSArray, operator_assign)
{
  RSA c1, c2;
  identity(c1, 10);
  c2 = c1;
  EXPECT_EQ(to_vector(c1), to_vector(c2));
}

TEST(RSArray, resize)
{
  size_t n = 10;
  RSA cycle;
  identity(cycle, n);
  EXPECT_EQ(cycle.size(), n);
  for (size_t i = 0; i < n; ++i) EXPECT_EQ(cycle[i], int(i));
  for (size_t i = 0; i < 1000; ++i)
  {
    cycle.resize(i);
    EXPECT_EQ(cycle.size(), i);
  }
}

TEST(RSArray, reverse)
{
  RSA cycle;
  identity(cycle, 4);
  cycle.reverse(1, 3);
  std::vector<int> A1 = { 0, 2, 1, 3 };
  EXPECT_EQ(to_vector(cycle), A1);

  identity(cycle, 10);
  cycle.reverse(2, 6);
  std::vector<int> A2 = { 0, 1, 5, 4, 3, 2, 6, 7, 8, 9 };
  EXPECT_EQ(to_vector(cycle), A2);
  cycle.reverse(0, 5);
  std::vector<int> A3 = { 3, 4, 5, 1, 0, 2, 6, 7, 8, 9 };
  EXPECT_EQ(to_vector(cycle), A3);
  cycle.reverse(8, 8);
  EXPECT_EQ(to_vector(cycle), A3);
  cycle.reverse(0, 10);
  std::vector<int> A4 = { 9, 8, 7, 6, 2, 0, 1, 5, 4, 3 };
  EXPECT_EQ(to_vector(cycle), A4);
}

TEST(RSArray, random)
{
  std::mt19937 random(623746283);
  size_t n = 100;
  RSA c1;
  std::vector<int> c2;
  identity(c1, n);
  identity(c2, n);
  for (size_t i = 0; i < 1000; ++i)
  {
    size_t l = random() % n, r = random() % (n - l + 1) + l;
    c1.reverse(l, r);
    cycle_reverse(c2, l, r);
    EXPECT_EQ(to_vector(c1), c2);
  }
}

