#ifndef MCTS_MONTECARLOTREE_H_
#define MCTS_MONTECARLOTREE_H_

#include <gtest/gtest_prod.h>

#include <cassert>
#include <limits>
#include <vector>
#include <utility>
#include <algorithm>
#include <memory>

namespace mcts
{
  typedef double Fitness;
  static const Fitness kMaxFitness = std::numeric_limits<Fitness>::infinity();

  template<typename Move, typename State, typename Policy>
  class MonteCarloTree;

  template<typename Move, class Policy> class Node
  {
      template<typename M, typename S, typename P> friend class MonteCarloTree;
      FRIEND_TEST(MonteCarloTreeTests, Node);
      FRIEND_TEST(MonteCarloTreeTests, Tree);
    private:
      typedef Node<Move, Policy> node_type;
      const Move move_;
      std::vector<node_type*> children_;
      typename Policy::Payload payload_;

    public:
      node_type& operator=(const node_type& other) = delete;
      explicit Node(const node_type& other) = delete;

      Node() : move_() {}

      explicit Node(const Move& move) : move_(move) {}
      ~Node()
      {
        for (auto n : children_) { delete n; }
      }

      typename Policy::Payload& operator()() { return payload_; }
      const typename Policy::Payload& operator()() const { return payload_; }

      node_type& operator[](ssize_t i) { return *children_.at(i); }
      const node_type& operator[](ssize_t i) const { return *children_.at(i); }

      size_t size() const { return children_.size(); }

      const Move& move() const { return move_; }

      bool is_leaf() const { return children_.empty(); }

      template<typename State> void expand(State &state)
      {
        assert(is_leaf());
        assert(!state.is_terminal());
        auto moves = state.moves();
        children_.resize(moves.size(), NULL);
        size_t i = 0;
        for (auto m : moves) { children_[i++] = new node_type(m); }
      }
  };

  template<typename Move, typename State, typename Policy> class MonteCarloTree
  {
      FRIEND_TEST(MonteCarloTreeTests, Tree);
    private:
      typedef Node<Move, Policy> node_type;
      typedef MonteCarloTree<Move, State, Policy> tree_type;
      Policy policy_;
      std::unique_ptr<node_type> root_;  // does not hold any move in fact
      State root_state_;

      Fitness playout(node_type &node, State &state,
          size_t iteration, size_t level)
      {
        if (node.is_leaf() && !state.is_terminal()
            && policy_.expand(node, state, iteration, level))
        {
          node.expand(state);
        }
        Fitness estimate;
        if (!node.is_leaf())
        {
          size_t chosen_idx = policy_.choose(node, state);
          assert(chosen_idx < node.size());
          node_type &chosen = *node.children_[chosen_idx];
          state.apply(chosen.move());
          estimate = playout(chosen, state, iteration, level + 1);
          policy_.update(node, chosen_idx, estimate);
        }
        else
        {
          estimate = state.estimate_playout(policy_.get_random());
          policy_.update(node, (ssize_t) (-1), estimate);
        }
        return estimate;
      }

    public:
      MonteCarloTree(const State& state, Policy policy) : policy_(policy),
        root_state_(state)
      {
        root_.reset(new node_type());
        root_->expand(root_state_);
      }

      template<typename ProgressCtrl>
      Move search(ProgressCtrl &progress_ctrl)
      {
        size_t iteration = 0;
        double progress = 0;
        Fitness best = kMaxFitness;
        while ((progress = progress_ctrl.progress(best)) <= 1)
        {
          State state = root_state_;
          Fitness estimate = playout(*root_.get(), state, iteration, 0);
          best = std::min(best, estimate);
          ++iteration;
        }
        size_t best_idx = policy_.best_child(*root_.get());
        assert(best_idx < root_->size());
        return root_->children_[best_idx]->move();
      }

      void apply(const Move& move)
      {
        for (node_type *&node : root_->children_)
        {
          if (move == node->move())
          {
            node_type* new_root = node;
            node = NULL;  // detach subtree before disposing entire tree
            root_.reset(new_root);  // this invokes old root destructor
            root_state_.apply(move);
            if (root_->is_leaf() && !root_state_.is_terminal())
            {
              root_->expand(root_state_);
            }
            break;
          }
        }
      }

      State &root_state()
      {
        return root_state_;
      }
  };
}

#endif  // MCTS_MONTECARLOTREE_H_
