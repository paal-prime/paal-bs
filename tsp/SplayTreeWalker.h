#ifndef TSP_SPLAYTREEWALKER_H_
#define TSP_SPLAYTREEWALKER_H_

#include <algorithm>
#include <cassert>

#include "tsp/SplayTree.h"
#include "tsp/util.h"

namespace tsp {

  template < typename Matrix, typename Random,
           enum SplayImplEnum SplayImpl = kTopDownUnbalanced >
  class SplayTreeWalker {
    public:
      typedef typename Matrix::value_type value_type;

      template<typename Cycle> SplayTreeWalker(const Matrix &matrix,
          Cycle &cycle, Random &random) : matrix_(matrix), random_(random),
        next_(cycle), best_(cycle) {
        assert(matrix_.size1() == matrix_.size2());
        assert(matrix_.size1() == cycle.size());
        best_fitness = next_fitness = fitness(matrix_, next_);
      }

      void prepare() {
        const size_t kN = matrix_.size1();
        split.generate(kN, random_);
        l_index_ = split.begin;
        r_index_ = split.end - 1;
        size_t l_prev_node = next_[l_index_ ? l_index_ - 1 : kN - 1],
               r_next_node = next_[r_index_ < kN - 1 ? r_index_ + 1 : 0],
               l_node = next_[l_index_],
               r_node = next_[r_index_];
        new_fitness = next_fitness
            - matrix_(l_prev_node, l_node)
            - matrix_(r_node, r_next_node)
            + matrix_(l_prev_node, r_node)
            + matrix_(l_node, r_next_node);
        if (new_fitness < best_fitness) {
          best_fitness = new_fitness;
        }
      }

      void step() {
        next_.reverse(l_index_, r_index_);
        next_fitness = new_fitness;
      }

      template<typename Cycle> void jump(const Cycle &cycle) {
        next_ = cycle;
        next_fitness = fitness(matrix_, next_);
        if (next_fitness < best_fitness) {
          best_fitness = next_fitness;
          best_ = next_;
        }
      }

      // TODO(stupaq) : fix in all walkers
      value_type best_fitness, next_fitness, new_fitness;

    private:
      Split split;
      const Matrix &matrix_;
      Random &random_;
      SplayTree<uint32_t, SplayImpl> next_;
      DynCycle best_;
      size_t l_index_, r_index_;
  };
}

#endif  // TSP_SPLAYTREEWALKER_H_

