#ifndef TSP_MCTS_H_
#define TSP_MCTS_H_

#include <cassert>
#include <limits>
#include <vector>
#include <utility>

namespace mcts {

  typedef double Fitness;

  template<typename Move> struct Node {
    typedef Node<Move> node_type;

    const Move move_;
    size_t visits_ = 0;
    Fitness estimate_ = 0;
    std::vector<std::unique_ptr<node_type> > children_;

    Node() {}
    explicit Node(const Move& move) : move_(move) {}

    bool is_leaf() const {
      return children_.empty();
    }

    template<typename Policy> void update(Policy& policy, Fitness estimate) {
      visits_++;
      estimate_ = policy.combine_estimate(estimate_, estimate, visits_);
    }

    template<typename State> void expand(const State& state) {
      if (is_leaf()) {
        auto moves = state.moves();
        children_.resize(moves.size());
        size_t i = 0;
        for (auto m : moves) {
          children_[i].reset(new node_type(m));
          ++i;
        }
      }
    }

    template<typename Stream, typename MoveType> friend
    Stream& operator<<(Stream& out, const Node<MoveType>& node) {
      out << node.move_ << "\tv:" << node.visits_ << "\te:"
          << node.estimate_ << "\tl:" << node.is_leaf();
      return out;
    }
  };

  template<typename Move, typename State, typename Policy>
  class MCTS {
    private:
      typedef Node<Move> node_type;

      Policy policy_;
      std::unique_ptr<node_type> root_;  // it does not hold any move in fact

      Fitness default_policy(State& state) {
        return state.default_playout(policy_.get_random());
      }

      template<typename Stack> State tree_policy(
        Stack& root_path) {
        State state(state_);
        node_type* node = root_.get();
        root_path.push_back(node);
        while (!node->is_leaf()) {
          node = policy_.best_child(node, state);
          root_path.push_back(node);
          state.apply(node->move_);
        }
        if (!state.is_terminal()
            && policy_.do_expand(node, state)) {
          node->expand(state);
          assert(!node->is_leaf());
          root_path.push_back(node);
          node = policy_.best_child(node, state);
          state.apply(node->move_);
        }
        return state;
      }

      template<typename Stack> void update_back(Stack& root_path,
          Fitness estimate) {
        while (!root_path.empty()) {
          node_type* node = root_path.back();
          root_path.pop_back();
          node->update(policy_, estimate);
        }
      }

    public:
      State state_;

      MCTS(const State& state, Policy policy) : policy_(policy), state_(state) {
        root_.reset(new node_type());
        root_->expand(state_);
      }

      Move search(size_t iterations) {
        while (iterations-- > 0) {
          std::vector<node_type*> root_path;
          State leaf = tree_policy(root_path);
          Fitness estimate = default_policy(leaf);
          assert(root_path.front() == root_.get());
          update_back(root_path, estimate);
        }
        Fitness best_fit = std::numeric_limits<Fitness>::infinity();
        node_type* best_node = NULL;
        for (auto& node : root_->children_) {
          if (node->estimate_ < best_fit) {
            best_fit = node->estimate_;
            best_node = node.get();
          }
        }
        assert(best_node != NULL);
        return best_node->move_;
      }

      void apply(const Move& move) {
        for (auto& node : root_->children_) {
          if (move == node->move_) {
            node_type* new_root = node.release();
            root_.reset(new_root);
            state_.apply(move);
            if (!state_.is_terminal()) {
              root_->expand(state_);
            }
            break;
          }
        }
      }

      template<typename Stream, typename M, typename S, typename P>
      friend Stream& operator<<(Stream& out, const MCTS<M, S, P>& tree) {
        out << "root: " << *tree.root_ << '\n';
        for (auto& node : tree.root_->children_) {
          out << *node << '\n';
        }
        return out;
      }
  };
}

#endif  // TSP_MCTS_H_
