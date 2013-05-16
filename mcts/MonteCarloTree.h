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

  /*
  concept State
  {
    concept Move;

    bool is_terminal() const;
    void apply(const Move &move);
    template<typename Random> Fitness estimate_playout(Random& random);
    const std::vector<Move> moves() const
  };
  */

  /** @brief Generic Monte Carlo Tree object, needs [mcts::Policy] for search
   * strategy and [mcts::State] for domain dependency
   */
  template<typename State, typename Policy> class MonteCarloTree
  {
      FRIEND_TEST(MonteCarloTreeTests, Node);
      FRIEND_TEST(MonteCarloTreeTests, Tree);
      typedef typename State::Move Move;
      typedef typename Policy::Payload Payload;
    public:
      class Node
      {
          friend class MonteCarloTree;
          FRIEND_TEST(MonteCarloTreeTests, Node);
          FRIEND_TEST(MonteCarloTreeTests, Tree);
        private:
          const Move move_;
          std::vector<std::unique_ptr<Node> > children_;
          Payload payload_;

        public:
          Node& operator=(const Node& other) = delete;
          explicit Node(const Node& other) = delete;

          Node() : move_() {}

          explicit Node(const Move& _move) : move_(_move) {}

          Payload& operator()() { return payload_; }
          const Payload& operator()() const { return payload_; }

          Node& operator[](ssize_t i) {
            assert(i >= 0 && i < (ssize_t) children_.size());
            return *children_[i].get();
          }
          const Node& operator[](ssize_t i) const {
            assert(i >= 0 && i < (ssize_t) children_.size());
            return *children_[i].get();
          }

          size_t best_child() const
          {
            size_t best = 0;
            for (size_t i = 0; i < size(); i++)
              if (children_[i]->payload_ < children_[best]->payload_) best = i;
            return best;
          }

          size_t size() const { return children_.size(); }

          const Move& move() const { return move_; }

          bool is_leaf() const { return children_.empty(); }

          void expand(State &state)
          {
            assert(is_leaf());
            auto moves = state.moves();
            assert(!moves.empty());
            children_.resize(moves.size());
            size_t i = 0;
            for (auto m : moves) { children_[i++].reset(new Node(m)); }
          }

          Fitness playout(Policy &policy, State &state, size_t iteration,
              size_t level)
          {
            if (is_leaf() && !state.is_terminal()
                && policy.expand(*this, level))
            {
              expand(state);
            }
            Fitness estimate;
            if (!is_leaf())
            {
              size_t chosen_idx = policy.choose(*this);
              assert(chosen_idx < size());
              Node &chosen = *children_[chosen_idx].get();
              state.apply(chosen.move());
              estimate = chosen.playout(policy, state, iteration, level + 1);
              policy.update(*this, chosen_idx, estimate);
            }
            else
            {
              estimate = state.estimate_playout(policy.get_random());
              policy.update(*this, (ssize_t) (-1), estimate);
            }
            return estimate;
          }
      };

    private:
      Policy policy_;
      std::unique_ptr<Node> root_;  // does not hold any move in fact
      State root_state_;

    public:
      /** @brief Creates search tree with given [mcts::Policy] and
       * [mcts::State] as a initial (root) state
       * @param state a [mcts::State] to be copied into the root of created search tree
       * @param policy a [mcts::Policy] determining tree behaviour
       **/
      MonteCarloTree(const State& state, const Policy &policy)
        : policy_(policy), root_state_(state)
      {
        root_.reset(new Node());
        root_->expand(root_state_);
      }

      /** @brief Performs search according to embedded [mcts::Policy],
       * [paal::ProgressCtrl] determines termination condition
       * @param progress_ctrl a termination condition
       **/
      template<typename ProgressCtrl>
      Move search(ProgressCtrl &progress_ctrl)
      {
        size_t iteration = 0;
        double progress = 0;
        Fitness best = std::numeric_limits<Fitness>::infinity();
        while ((progress = progress_ctrl.progress(best)) <= 1)
        {
          State state = root_state_;
          Fitness estimate = root_->playout(policy_, state, iteration, 0);
          best = std::min(best, estimate);
          ++iteration;
        }
        size_t best_idx = root_->best_child();
        assert(best_idx < root_->size());
        return root_->children_[best_idx]->move();
      }

      /** @brief Applies given [mcts::State::Move], removes inaccssible part of
       * the tree, statistics obtaine for preserved part are not removed
       * @param move a [mcts::State::Move] to apply
       **/
      void apply(const Move& move)
      {
        for (auto &node : root_->children_)
        {
          if (move == node->move())
          {
            Node* new_root = node.release();
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

      /** @brief Root statee accessor
       * @returns reference to the root state
       **/
      State &root_state() { return root_state_; }
  };
}  // namespace mcts

#endif  // MCTS_MONTECARLOTREE_H_
