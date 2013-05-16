#ifndef TSP_MCTS_TSP_H_
#define TSP_MCTS_TSP_H_

#include <limits>
#include <algorithm>
#include <cmath>
#include <vector>
#include <numeric>
#include <functional>
#include <utility>

namespace tsp
{
  using mcts::Fitness;

  /** @brief [implements mcts::State]
   * TSP cycle building state for MCTS tree with accurate (exhaustive search)
   * estimates for small subtrees
   **/
  template<typename Matrix> class TSPState
  {
    public:
      typedef size_t Move;

    private:
      const Matrix& matrix_;
      const size_t exhaustive_limit_;
      const size_t moves_limit_;
      size_t left_count_;
      size_t first_vertex_;
      size_t last_vertex_;
      std::vector<bool> in_path_;

      struct MovesComparator
      {
        size_t last_;
        const Matrix& matrix_;

        MovesComparator(size_t last, const Matrix& matrix) : last_(last),
          matrix_(matrix) {}

        bool operator()(size_t v1, size_t v2)
        { return matrix_(last_, v1) < matrix_(last_, v2); }
      };

      template<typename Combine>
      Fitness exhaustive_accumulate(Combine combine, Fitness initial)
      {
        auto left_vertices = moves_all();
        // assertion: left_vertices vector is sorted in asceding order
        Fitness accumulator = initial;
        do
        {
          Fitness cost = cost_;
          size_t last = last_vertex_;
          for (auto v : left_vertices)
          {
            cost += matrix_(last, v);
            last = v;
          }
          cost += matrix_(last, first_vertex_);
          accumulator = combine(accumulator, cost);
        }
        while (std::next_permutation(left_vertices.begin(),
              left_vertices.end()));
        return accumulator;
      }

      const std::vector<Move> moves_all() const
      {
        std::vector<Move> ms;
        for (size_t i = 0; i < in_path_.size(); i++)
        {
          if (!in_path_[i]) { ms.push_back(Move(i)); }
        }
        assert(ms.size() == left_count_);
        return ms;
      }

    public:
      Fitness cost_;

      /** @brief Creates initial state for TSP instance
       * @param matrix a [tsp::Matrix] holding distances between vertices
       * @param moves_limit maximal number of moves to return by moves() call
       * @param exhaustive_limit maximal number of moves left when
       * extimate_playout is allowed to start exhaustive search
       **/
      explicit TSPState(
          const Matrix& matrix,
          size_t moves_limit = std::numeric_limits<size_t>::max(),
          size_t exhaustive_limit = 4) :
        matrix_(matrix),
        exhaustive_limit_(exhaustive_limit),
        moves_limit_(moves_limit),
        left_count_(matrix.size1() - 1),
        first_vertex_(matrix.size1() - 1),
        last_vertex_(first_vertex_),
        cost_(0)
      {
        in_path_.resize(matrix.size1(), false);
        in_path_[first_vertex_] = true;
      }

      /** @brief Determines whether state is terminal */
      bool is_terminal() const { return left_count_ == 0; }

      /** @brief Mutates state according to provided Move
       * @param move a Move to apply
       **/
      void apply(const Move& move)
      {
        assert(!is_terminal());
        assert(!in_path_[move]);
        in_path_[move] = true;
        left_count_--;
        cost_ += matrix_(last_vertex_, move);
        last_vertex_ = move;
        if (is_terminal()) { cost_ += matrix_(last_vertex_, first_vertex_); }
      }

      /** @brief Estimates objective function for current state
       * @param random a random number generator compliant with C++11 random concept
       **/
      template<typename Random> Fitness estimate_playout(Random& random)
      {
        if (!is_terminal())
        {
          auto left_moves = moves_all();
          std::shuffle(left_moves.begin(), left_moves.end(), random);
          for (auto m : left_moves)
          {
            if (left_decisions() < exhaustive_limit_)
            {
              exhaustive_search_min();
              break;
            }
            apply(m);
          }
        }
        return cost_;
      }

      /** @brief Enumerates moves allowed from current State
       * @returns a vector of Moves
       **/
      const std::vector<Move> moves() const
      {
        assert(!is_terminal());
        std::vector<Move> ms = moves_all();
        if (ms.size() > moves_limit_)
        {
          std::sort(ms.begin(), ms.end(),
              MovesComparator(last_vertex_, matrix_));
          ms.resize(moves_limit_);
        }
        return ms;
      }

      /** @brief Makes an exhaustive search and returns minimal estimate from
       * estimates of all states reachable from current one by applying finite
       * number of moves
       * @returns estimate of objective function
       **/
      void exhaustive_search_min()
      {
        assert(!is_terminal());
        Fitness best_cost = exhaustive_accumulate(
              [](Fitness a, Fitness b) -> Fitness { return (a < b) ? a : b; },
              std::numeric_limits<Fitness>::infinity());
        cost_ = best_cost;
        // we're not reproducing moves sequence but making state terminal
        left_count_ = 0;
        assert(is_terminal());
      }

      /** @brief Estimates how many Moves must be applied in order toread
       * terminal state
       * @returns number of moves to terminal state
       **/
      size_t left_decisions() const { return left_count_; }
  };
}  // namespace tsp

#endif  // TSP_MCTS_TSP_H_
