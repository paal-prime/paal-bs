#ifndef TSP_MCTS_TSP_H_
#define TSP_MCTS_TSP_H_

#include <tsp/MCTS.h>
#include <limits>
#include <cmath>
#include <algorithm>
#include <vector>
#include <functional>
#include <utility>

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
      if (move.vertex_ != kNoVertex) {
        out << move.vertex_;
      } else {
        out << "no_vertex";
      }
      return out;
    }
  };

  template<typename Matrix> class TSPState {
      template<typename T> struct min {
        const T& operator() (const T& a, const T& b) const {
          return (a < b) ? a : b;
        }
      };

    private:
      const Matrix& matrix_;
      size_t left_count_;
      size_t first_vertex_;
      size_t last_vertex_;
      std::vector<bool> in_path_;

      template<typename Combine>
      Fitness exhaustive_accumulate(Combine combine, Fitness initial) {
        auto left_vertices = moves<size_t>();
        // assertion: left_vertices is sorted asceding
        Fitness accumulator = initial;
        do {
          Fitness cost = cost_;
          size_t last = last_vertex_;
          for (auto v : left_vertices) {
            cost += matrix_(last, v);
            last = v;
          }
          cost += matrix_(last, first_vertex_);
          accumulator = combine(accumulator, cost);
          min_found_ = std::min(min_found_, cost);
        } while (std::next_permutation(left_vertices.begin(),
              left_vertices.end()));
        return accumulator;
      }

    public:
      Fitness cost_;
      Fitness min_found_;

      explicit TSPState(const Matrix& matrix) : matrix_(matrix),
      left_count_(matrix.size1() - 1), first_vertex_(matrix.size1() - 1),
      last_vertex_(first_vertex_), cost_(0),
      min_found_(std::numeric_limits<double>::infinity()) {
        in_path_.resize(matrix.size1(), false);
        in_path_[first_vertex_] = true;
      }

      bool is_terminal() const {
        return left_count_ == 0;
      }

      void apply(const TSPMove& move) {
        assert(!is_terminal());
        assert(!in_path_[move.vertex_]);
        in_path_[move.vertex_] = true;
        left_count_--;
        cost_ += matrix_(last_vertex_, move.vertex_);
        last_vertex_ = move.vertex_;
        if (is_terminal()) {
          cost_ += matrix_(last_vertex_, first_vertex_);
        }
      }

      template<typename Random>
      std::pair<Fitness, Fitness> default_playout(Random& random) {
        if (!is_terminal()) {
          auto left_moves = moves();
          std::shuffle(left_moves.begin(), left_moves.end(), random);
          for (auto m : left_moves) {
            if (moves_count() < 4) {
              exhaustive_search_min();
              break;
            }
            apply(m);
          }
        }
        assert(is_terminal());
        return std::make_pair(cost_, min_found_);
      }

      template<typename Move = TSPMove> const std::vector<Move> moves() const {
        assert(!is_terminal());
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
        return left_count_;
      }

      void exhaustive_search_min() {
        assert(!is_terminal());
        Fitness best_cost = exhaustive_accumulate(min<Fitness>(),
            std::numeric_limits<Fitness>::infinity());
        cost_ = best_cost;
        // we're not reproducing moves sequence but making state terminal
        left_count_ = 0;
        assert(is_terminal());
      }
  };

  template<typename Random = std::mt19937> class TSPPolicyLCB {
      const double kLCBParam = 1.0;

    private:
      Random& random_;

    public:
      explicit TSPPolicyLCB(Random& random) : random_(random) {
      }

      template<typename Node>
      void update(Node* node, Fitness sample) {
        if (node->visits_ == 1) {
          node->estimate_ = sample;
        } else {
          node->estimate_ = (node->estimate_ * (node->visits_ - 1) + sample) /
            node->visits_;
        }
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

  template<typename Random = std::mt19937> class TSPPolicyRND {
    private:
      Random& random_;

    public:
      explicit TSPPolicyRND(Random& random) : random_(random) {
      }

      template<typename Node>
      void update(Node* node, Fitness sample) {
        if (node->visits_ == 1) {
          node->estimate_ = sample;
        } else {
          node->estimate_ = (node->estimate_ * (node->visits_ - 1) + sample) /
            node->visits_;
        }
      }

      template<typename Node, typename State>
      Node* best_child(Node* parent, const State& state) {
        assert(!parent->children_.empty());
        Node* pick = parent->children_[random_() %
          parent->children_.size()].get();
        assert(pick != NULL);
        return pick;
      }

      std::mt19937& get_random() {
        return random_;
      }

      template<typename Node, typename State> bool do_expand(const Node* node,
          const State& state) {
        return node->visits_ >= state.moves_count();
      }
  };

  template<typename Random = std::mt19937> class TSPPolicyRNDeBest {
      const double bestPickProbability = 0.05;

    private:
      Random& random_;

    public:
      explicit TSPPolicyRNDeBest(Random& random) : random_(random) {
      }

      template<typename Node>
      void update(Node* node, Fitness sample) {
        if (node->visits_ == 1) {
          node->estimate_ = sample;
        } else {
          node->estimate_ = (node->estimate_ * (node->visits_ - 1) + sample) /
            node->visits_;
        }
      }

      template<typename Node, typename State>
      Node* best_child(Node* parent, const State& state) {
        assert(!parent->children_.empty());
        Node* pick = NULL;
        if (parent->best_node_ != NULL
            && bestPickProbability >= static_cast<double>(random_()) /
              random_.max()) {
          pick = parent->best_node_;
        } else {
          pick = parent->children_[random_() % parent->children_.size()].get();
        }
        assert(pick != NULL);
        return pick;
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
