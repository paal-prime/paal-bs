#ifndef TSP_MCTS_TSP_H_
#define TSP_MCTS_TSP_H_

#include <mcts/MonteCarloTree.h>

#include <limits>
#include <cmath>
#include <algorithm>
#include <vector>
#include <functional>
#include <utility>

namespace tsp
{
  typedef mcts::Fitness Fitness;

  typedef size_t TSPMove;

  template<typename Matrix> class TSPState
  {
    private:
      const Matrix& matrix_;
      size_t left_count_;
      size_t first_vertex_;
      size_t last_vertex_;
      std::vector<bool> in_path_;

      template<typename Combine>
      Fitness exhaustive_accumulate(Combine combine, Fitness initial)
      {
        auto left_vertices = moves<size_t>();
        // assertion: left_vertices is sorted asceding
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

    public:
      Fitness cost_;

      explicit TSPState(const Matrix& matrix) : matrix_(matrix),
        left_count_(matrix.size1() - 1), first_vertex_(matrix.size1() - 1),
        last_vertex_(first_vertex_), cost_(0)
      {
        in_path_.resize(matrix.size1(), false);
        in_path_[first_vertex_] = true;
      }

      bool is_terminal() const
      {
        return left_count_ == 0;
      }

      void apply(const TSPMove& move)
      {
        assert(!is_terminal());
        assert(!in_path_[move]);
        in_path_[move] = true;
        left_count_--;
        cost_ += matrix_(last_vertex_, move);
        last_vertex_ = move;
        if (is_terminal())
        {
          cost_ += matrix_(last_vertex_, first_vertex_);
        }
      }

      template<typename Random> Fitness estimate_playout(Random& random)
      {
        if (!is_terminal())
        {
          auto left_moves = moves();
          std::shuffle(left_moves.begin(), left_moves.end(), random);
          for (auto m : left_moves)
          {
            if (moves_count() < 4)
            {
              exhaustive_search_min();
              break;
            }
            apply(m);
          }
        }
        assert(is_terminal());
        return cost_;
      }

      template<typename Move = TSPMove> const std::vector<Move> moves() const
      {
        assert(!is_terminal());
        std::vector<Move> ms;
        for (size_t i = 0; i < in_path_.size(); i++)
        {
          if (!in_path_[i])
          {
            ms.push_back(Move(i));
          }
        }
        assert(ms.size() == moves_count());
        return ms;
      }

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

      size_t moves_count() const
      {
        return left_count_;
      }
  };

  template<typename Random = std::mt19937> class TSPPolicyRND
  {
    private:
      Random& random_;

    public:
      typedef struct
      {
        size_t visits_ = 0;
        Fitness estimate_ = std::numeric_limits<Fitness>::max();
      } Payload;

      explicit TSPPolicyRND(Random& random) : random_(random) {}

      std::mt19937& get_random()
      {
        return random_;
      }

      template<typename Node>
      void update(Node& parent, ssize_t chosen, Fitness estimate)
      {
        Payload& payload = parent();
        payload.visits_++;
        payload.estimate_ = (payload.visits_ == 1)
          ? estimate
          : payload.estimate_ = (payload.estimate_ * payload.visits_ + estimate)
              / (payload.visits_ + 1);
      }

      template<typename Node, typename State> size_t choose(const Node& parent,
          const State &state)
      {
        assert(!parent.is_leaf());
        return (random_() % parent.size());
      }

      template<typename Node> size_t best_child(const Node &parent)
      {
        assert(!parent.is_leaf());
        Fitness best = std::numeric_limits<Fitness>::max();
        ssize_t index = -1;
        for (size_t i = 0; i < parent.size(); i++)
        {
          if (best > parent[i]().estimate_)
          {
            best = parent[i]().estimate_;
            index = i;
          }
        }
        assert(index >= 0);
        return static_cast<size_t>(index);
      }

      template<typename Node, typename State> bool expand(const Node& node,
          const State& state, size_t iteration, size_t level)
      {
        return node().visits_ >= state.moves_count();
      }
  };
}

#endif  // TSP_MCTS_TSP_H_
