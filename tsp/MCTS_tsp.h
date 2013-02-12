#ifndef TSP_MCTS_TSP_H_
#define TSP_MCTS_TSP_H_

#include <tsp/MCTS.h>
#include <limits>
#include <cmath>
#include <algorithm>
#include <vector>

namespace tsp {

  typedef mcts::Fitness Fitness;
  static const size_t kNoVertex = -1;

  struct TSPMove {
    size_t vertex_;

    TSPMove() : vertex_(kNoVertex) {}
    explicit TSPMove(size_t vertex) : vertex_(vertex) {}

    bool operator==(const TSPMove& other) const {
      return vertex_ == other.vertex_;
    }

    template<typename Stream> friend
    Stream& operator<<(Stream& out, const TSPMove& move) {
      out << move.vertex_;
      return out;
    }
  };

  template<typename Matrix> class TSPState {
    private:
      const Matrix& matrix_;
      size_t length_;
      size_t first_vertex_;
      size_t last_vertex_;
      std::vector<bool> in_path_;

    public:
      Fitness cost_;

      explicit TSPState(const Matrix& matrix) : matrix_(matrix),
        length_(0), last_vertex_(kNoVertex), cost_(0) {
        in_path_.resize(matrix.size1(), false);
      }

      bool is_terminal() const {
        return length_ == in_path_.size();
      }

      void apply(const TSPMove& move) {
        assert(!is_terminal());
        assert(!in_path_[move.vertex_]);
        in_path_[move.vertex_] = true;
        length_++;
        if (last_vertex_ != kNoVertex) {
          cost_ += matrix_(last_vertex_, move.vertex_);
        } else {
          first_vertex_ = move.vertex_;
        }
        last_vertex_ = move.vertex_;
        if (is_terminal()) {
          cost_ += matrix_(last_vertex_, first_vertex_);
        }
      }

      template<typename Random> Fitness default_playout(Random& random) {
        auto left_moves = moves();
        std::shuffle(left_moves.begin(), left_moves.end(), random);
        for (auto m : left_moves) {
          if (moves_count() < 5) {  // TODO(stupaq): this makes things worse
            exhaustive_search();
            break;
          }
          apply(m);
        }
        assert(is_terminal());
        return cost_;
      }

      template<typename Move = TSPMove> const std::vector<Move> moves() const {
        std::vector<Move> ms;
        for (size_t i = 0; i < in_path_.size(); i++) {
          if (!in_path_[i]) {
            ms.push_back(Move(i));
          }
        }
        assert(ms.size() == moves_count());
        return ms;
      }

      size_t moves_count() const {
        return in_path_.size() - length_;
      }

      void exhaustive_search() {
        assert(!is_terminal());
        assert(length_ > 0);
        auto left_vertices = moves<size_t>();
        // assertion: path is sorted
        Fitness best_cost = std::numeric_limits<Fitness>::infinity();
        do {
          size_t last = last_vertex_;
          Fitness cost = 0;
          for (auto v : left_vertices) {
            cost += matrix_(last, v);
            last = v;
          }
          cost += matrix_(last, first_vertex_);
          if (cost < best_cost) {
            best_cost = cost;
          }
        } while(std::next_permutation(left_vertices.begin(),
              left_vertices.end()));
        cost_ += best_cost;
        // we're not reproducing moves sequence but making state terminal
        length_ = in_path_.size();
        for (auto v : left_vertices) {
          in_path_[v] = true;
        }
        assert(is_terminal());
      }
  };

  template<typename Random> class TSPPolicy {
      const double kLCBParam = 1.0;

    private:
      Random& random_;

    public:
      explicit TSPPolicy(Random& random) : random_(random) {
      }

      Fitness combine_estimate(Fitness estimate, Fitness sample,
          size_t visits) {
        return (visits == 1) ? sample :
          (estimate * (visits - 1) + sample) / visits;
      }

      template<typename Node, typename State>
      Node* best_child(Node* parent, const State& state) {
        Fitness best_est = std::numeric_limits<Fitness>::infinity();
        Node* best_node = NULL;
        assert(!parent->children_.empty());
        for (auto& node : parent->children_) {
          Fitness lcb = node->estimate_;
          if (parent->visits_ == 0 || node->visits_ == 0) {
            lcb -= std::numeric_limits<Fitness>::infinity();
          } else {
            lcb *= 1 - kLCBParam * sqrt(log(parent->visits_) / node->visits_);
          }
          if (lcb < best_est) {
            best_est = lcb;
            best_node = node.get();
          }
        }
        assert(best_node != NULL);
        return best_node;
      }

      std::mt19937& get_random() {
        return random_;
      }

      template<typename Node, typename State> bool do_expand(const Node* node,
          const State& state) {
        return node->visits_ >= state.moves_count();
      }
  };
}

#endif  // TSP_MCTS_TSP_H_
