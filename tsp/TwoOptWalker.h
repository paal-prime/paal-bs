#ifndef TSP_TWOOPTWALKER_H_
#define TSP_TWOOPTWALKER_H_

// http://en.wikipedia.org/wiki/2-opt

#include <cassert>
#include <vector>

#include "tsp/util.h"

namespace tsp
{
  template<typename Matrix> struct TwoOptWalker  // implements Walker
  {
    /*template<typename Random>
    TwoOptWalker(const Matrix &_matrix, Random &random) : matrix(_matrix)
    {
      assert(matrix.size1() == matrix.size2());
      cycle_shuffle(cycle, matrix.size1(), random);
    }*/

    template<typename Cycle>
    TwoOptWalker(const Matrix &_matrix, const Cycle &_cycle) :
        matrix(_matrix), cycle(_cycle)
    {
      assert(matrix.size1() == matrix.size2() &&
             matrix.size1() == _cycle.size());
      current_fitness_ = fitness(matrix, cycle);
    }

    const Matrix &matrix;
    double current_fitness_, next_fitness_;
    std::vector<size_t> cycle;
    Split split;

    double current_fitness()
    {
      return current_fitness_;
    }
    double next_fitness()
    {
      return next_fitness_;
    }

    template<typename Random> void prepare_step(double progress, Random &random)
    {
      size_t n = matrix.size1();
      split.generate(n, random);
      size_t lp = split.begin ? split.begin - 1 : n - 1;
      size_t rn = split.end < n ? split.end : 0;
      next_fitness_ = current_fitness_
          - matrix(cycle[lp], cycle[split.begin])
          - matrix(cycle[split.end-1], cycle[rn])
          + matrix(cycle[lp], cycle[split.end-1])
          + matrix(cycle[split.begin], cycle[rn]);
    }

    void make_step()
    {
      cycle_reverse(cycle, split.begin, split.end);
      current_fitness_ = next_fitness_;
    }
  };
}

#endif  // TSP_TWOOPTWALKER_H_

