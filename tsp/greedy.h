#ifndef TSP_GREEDY_H_
#define TSP_GREEDY_H_

#include <vector>

#include "tsp/util.h"

namespace tsp
{
  /** @brief greedy O(log n) apx
   * see: http://link.springer.com/chapter/10.1007%2F978-1-4020-9688-4_3
   */
  template<typename Matrix, typename Cycle>
  void greedy(const Matrix &matrix, Cycle &cycle)
  {
    size_t n = matrix.size1();
    cycle.resize(n);
    std::vector<bool> V(n, 0);
    cycle[0] = 0;
    V[0] = 1;
    for (size_t i = 1; i < n; ++i)
    {
      size_t bi = 0;
      for (size_t j = 0; j < n; ++j)
        if (!V[j] &&
            (!bi || matrix(cycle[i - 1], j) < matrix(cycle[i - 1], bi)))
          bi = j;
      cycle[i] = bi;
      V[bi] = 1;
    }
  }
}  // namespace tsp

#endif  // TSP_GREEDY_H_
