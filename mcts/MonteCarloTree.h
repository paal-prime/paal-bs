#ifndef MCTS_MONTECARLOTREE_H_
#define MCTS_MONTECARLOTREE_H_

#include <gtest/gtest.h>

#include <cassert>
#include <limits>
#include <vector>
#include <utility>
#include <algorithm>
#include <memory>

namespace mcts {

  typedef double Fitness;

  template<typename Move> struct Node {
    typedef Node<Move> node_type;

    const Move move_;
    size_t visits_ = 0;
    Fitness estimate_ = 0;
    node_type* best_node_ = NULL;
    std::vector<std::unique_ptr<node_type> > children_;

    Node() : move_() {}
    explicit Node(const Move& move) : move_(move) {}

    bool is_leaf() const {
      return children_.empty();
    }

    template<typename PolicyType>
    void update(PolicyType& policy, Fitness estimate, node_type* previous) {
      visits_++;
      estimate_ = policy.update(visits_, estimate_, estimate);
      if (previous != NULL) {
        if (best_node_ == NULL || best_node_->estimate_ > previous->estimate_) {
          best_node_ = previous;
        }
      }
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

    template<typename Stream> friend
    Stream& operator<<(Stream& out, const Node<Move>& node) {
      out << node.move_ << "\tl:" << node.is_leaf();
      return out;
    }
  };

  template<typename Move, typename State, typename Policy>
  class MonteCarloTree {
      FRIEND_TEST(MonteCarloTreeTest, Tree);
    private:
      typedef Node<Move> node_type;

      Policy policy_;
      std::unique_ptr<node_type> root_;  // it does not hold any move in fact

      Fitness default_policy(State& state) {
        auto found = state.default_playout(policy_.get_random());
        Fitness fit = found.first, best = found.second;
        best_found_ = std::min(best_found_, best);
        return fit;
      }

      template<typename Stack> State tree_policy(
        Stack& root_path) {
        State state(state_);
        node_type* node = root_.get();
        while (!node->is_leaf()) {
          root_path.push_back(node);
          node = policy_.best_child(node, state);
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
        root_path.push_back(node);
        return state;
      }

      template<typename Stack> void update_back(Stack& root_path,
          Fitness estimate) {
        node_type* previous, * node = NULL;
        while (!root_path.empty()) {
          previous = node;
          node = root_path.back();
          root_path.pop_back();
          node->update(policy_, estimate, previous);
        }
      }

    public:
      State state_;
      Fitness best_found_;

      MonteCarloTree(const State& state, Policy policy) : policy_(policy),
      state_(state), best_found_(std::numeric_limits<Fitness>::infinity()) {
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
        assert(root_->best_node_ != NULL);
        return root_->best_node_->move_;
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

      template<typename Stream> friend Stream&
      operator<<(Stream& out, const MonteCarloTree<Move, State, Policy>& tree) {
        out << "root: " << *tree.root_ << '\n';
        for (auto& node : tree.root_->children_) {
          out << *node << '\n';
        }
        return out;
      }
  };
}

#endif  // MCTS_MONTECARLOTREE_H_
