#include <gtest/gtest.h>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <climits>
#include <algorithm>

#include "tsp/SplayTree.h"

using tsp::SplayTree;

template<typename T, typename I> bool check_content(T &t, I begin, I end) { // NOLINT
  if (!std::equal(begin, end, t.begin())) {
    return false;
  }
  std::reverse(begin, end);
  if (!std::equal(begin, end, t.rbegin())) {
    return false;
  }
  std::reverse(begin, end);
  for (int i = 0; begin != end; i++, begin++) {
    if (*begin != t[i]) {
      return false;
    }
  }
  return true;
}

template<typename T, typename I> bool check_random_splay(T &t, I begin, // NOLINT
    I end, const size_t kM) {
  const size_t kN = end - begin;
  for (size_t i = 0; i < kM; i++) {
    int el = rand() % kN; // NOLINT
    if (*(begin + el) != t.splay(el)->val_) {
      return false;
    }
  }
  if (!check_content(t, begin, end)) {
    return false;
  }
  return true;
}

template<typename I> void fill_range(I begin, I end) {
  int i = 0;
  for (I iter = begin; iter != end; iter++) {
    *iter = i;
    i++;
  }
}

template<typename T, typename I> void reverse_range(T &t, I begin, // NOLINT
    size_t rbegin, size_t rend) {
  std::reverse(begin + rbegin, begin + rend + 1);
  t.reverse(rbegin, rend);
}

TEST(SplayTree, CreateEmpty) {
  SplayTree<int> t;
  ASSERT_EQ(0, t.size());
  ASSERT_TRUE(t.empty());
  int *input = NULL;
  ASSERT_TRUE(check_content(t, input, input));
}

TEST(SplayTree, CreateFromRange) {
  const int kN = 101;
  int input[kN];
  fill_range(input, input + kN);
  SplayTree<int> t(input, input + kN);
  ASSERT_TRUE(check_content(t, input, input + kN));
}

TEST(SplayTree, Find) {
  const int kN = 1001;
  int input[kN];
  fill_range(input, input + kN);
  SplayTree<int> t(input, input + kN);
  ASSERT_TRUE(check_content(t, input, input + kN));
  for (int i = 0; i < kN; i++) {
    ASSERT_EQ(input[i], t[i]);
  }
}

TEST(SplayTree, Splay) {
  srand(123433);
  const int kN = 131, kM = 1701;
  int input[kN];
  fill_range(input, input + kN);
  SplayTree<int> t(input, input + kN);
  ASSERT_TRUE(check_content(t, input, input + kN));
  ASSERT_TRUE(check_random_splay(t, input, input + kN, kM));
}

TEST(SplayTree, SplitHigher) {
  srand(123433);
  const int kN = 10435, kL = 47;
  int input[kN];
  fill_range(input, input + kN);
  SplayTree<int> t(input, input + kN);
  ASSERT_TRUE(check_content(t, input, input + kN));
  const int k1 = kN / 2, k2 = kN / 4, k3 = kN / 8;
  SplayTree<int> t1 = t.split_higher(k1);
  ASSERT_TRUE(check_random_splay(t, input, input + k1 + 1, kL));
  ASSERT_TRUE(check_random_splay(t1, input + k1 + 1, input + kN, kL));
  SplayTree<int> t2 = t.split_higher(k2);
  ASSERT_TRUE(check_random_splay(t, input, input + k2 + 1, kL));
  ASSERT_TRUE(check_random_splay(t2, input + k2 + 1, input + k1 + 1, kL));
  SplayTree<int> t3 = t.split_higher(k3);
  ASSERT_TRUE(check_random_splay(t, input, input + k3 + 1, kL));
  ASSERT_TRUE(check_random_splay(t3, input + k3 + 1, input + k2 + 1, kL));
}

TEST(SplayTree, SplitLower) {
  srand(123433);
  const int kN = 10435, kL = 47;
  int input[kN];
  fill_range(input, input + kN);
  SplayTree<int> t(input, input + kN);
  ASSERT_TRUE(check_content(t, input, input + kN));
  const int k1 = kN / 2, k2 = kN / 4, k3 = kN / 8;
  SplayTree<int> t1 = t.split_lower(k1);
  ASSERT_TRUE(check_random_splay(t, input + k1, input + kN, kL));
  ASSERT_TRUE(check_random_splay(t1, input, input + k1, kL));
  SplayTree<int> t2 = t.split_lower(k2);
  ASSERT_TRUE(check_random_splay(t, input + k1 + k2, input + kN, kL));
  ASSERT_TRUE(check_random_splay(t2, input + k1, input + k1 + k2, kL));
  SplayTree<int> t3 = t.split_lower(k3);
  ASSERT_TRUE(check_random_splay(t, input + k1 + k2 + k3, input + kN, kL));
  ASSERT_TRUE(check_random_splay(t3, input + k1 + k2,
                                 input + k1 + k2 + k3, kL));
}

TEST(SplayTree, Merge) {
  srand(123433);
  const int kN = 10382, kM = 3875, kL = 51;
  ASSERT_TRUE(kN > kM);
  int input[kN];
  fill_range(input, input + kN);
  SplayTree<int> t1(input, input + kM);
  ASSERT_TRUE(check_content(t1, input, input + kM));
  SplayTree<int> t2(input + kM, input + kN);
  ASSERT_TRUE(check_content(t2, input + kM, input + kN));
  t1.merge_right(t2);
  ASSERT_TRUE(check_random_splay(t1, input, input + kN, kL));
}

TEST(SplayTree, Reverse) {
  srand(123433);
  const int kN = 231, kM = 2000, kL = 47;
  int input[kN];
  fill_range(input, input + kN);
  SplayTree<int> t(input, input + kN);
  ASSERT_TRUE(check_content(t, input, input + kN));
  for (int i = 0; i < kM; i++) {
    int a = rand() % kN; // NOLINT
    int b = (a < kN) ? rand() % (kN - a) : 0; // NOLINT
    reverse_range(t, input, a, a + b);
    ASSERT_TRUE(check_random_splay(t, input, input + kN, kL));
  }
}

TEST(SplayTree, ReversePerformance) {
  srand(123433);
  const int kN = 104729, kM = 26669, kP = 5;
  int input[kN];
  int *end = input + kN;
  fill_range(input, end);
  SplayTree<int> t(input, end);
  boost::random::mt19937 gen;
  boost::random::uniform_int_distribution<> dist(0, kN - 1);
  for (int i = 0, lp = 0; i < kM; i++) {
    int a = dist(gen);
    int b = (a < kN) ? dist(gen) % (kN - a) : 0;
    reverse_range(t, input, a, a + b);
    if (kP) {
      int cp = (100 * i) / kM;
      if (cp >= lp) {
        std::cerr << cp << "%\n";
        lp += kP;
      }
    }
  }
  if (kP) {
    std::cerr << "100%\n";
  }
  ASSERT_TRUE(check_content(t, input, end));
}
