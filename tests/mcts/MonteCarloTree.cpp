#include <gtest/gtest.h>

#include <mcts/MonteCarloTree.h>
#include <tsp/TSPLIB.h>
#include <tsp/MCTS_tsp.h>

#include <algorithm>
#include <vector>
#include <utility>
#include <algorithm>

typedef int TestMove;

struct TestState {
  std::vector<TestMove> moves_ = {0, 1, 2, 3, 4, 5};

  const std::vector<TestMove> moves() const {
    return moves_;
  }

  void apply(const TestMove& move) {
    moves_.erase(std::find(moves_.begin(), moves_.end(), move));
  }

  bool is_terminal() {
    return moves_.empty();
  }

  std::pair<double, double> default_playout(std::mt19937& random) {
    moves_.empty();
    return std::make_pair(100, 50);
  }
};

struct TestPolicy {
  std::mt19937 random_;

  double update(size_t visits, double estimate, double sample) {
    return (visits == 1) ? sample : std::min(estimate, sample);
  }

  template<typename Node, typename State> bool do_expand(const Node* node,
      const State& state) {
    return true;
  }

  template<typename Node>
  Node* best_child(Node* parent, const TestState& state) {
    assert(parent->children_.size() > 0);
    return parent->children_[0].get();
  }

  std::mt19937& get_random() {
    return random_;
  }
};

class MonteCarloTreeTests : public testing::Test {
};

TEST_F(MonteCarloTreeTests, Node) {
  using mcts::Node;

  Node<TestMove> node(-1);
  TestState state;
  ASSERT_GT(state.moves().size(), 2);
  node.expand(state);
  ASSERT_EQ(state.moves().size(), node.children_.size());
  for (size_t i = 0; i < state.moves().size(); i++) {
    ASSERT_EQ(state.moves_[i], node.children_[i]->move_);
  }
  ASSERT_EQ(NULL, node.best_node_);
  TestPolicy policy;

  Node<TestMove>* chld0 = node.children_[0].get();
  chld0->update(policy, 100, NULL);
  ASSERT_EQ(NULL, chld0->best_node_);
  ASSERT_EQ(100, chld0->estimate_);
  node.update(policy, 100, chld0);
  ASSERT_EQ(chld0, node.best_node_);
  ASSERT_EQ(100, node.estimate_);

  Node<TestMove>* chld2 = node.children_[2].get();
  chld2->update(policy, 50, NULL);
  ASSERT_EQ(NULL, chld2->best_node_);
  ASSERT_EQ(50, chld2->estimate_);
  node.update(policy, 50, chld2);
  ASSERT_EQ(chld2, node.best_node_);
  ASSERT_EQ(50, node.estimate_);
}

TEST_F(MonteCarloTreeTests, Tree) {
  using mcts::MonteCarloTree;

  TestState state;
  TestPolicy policy;
  MonteCarloTree<TestMove, TestState, TestPolicy> tree(state, policy);
  TestMove move;
  move = tree.search(50);
  ASSERT_EQ(0, move);
  ASSERT_EQ(50, tree.best_found_);
  tree.apply(move);
  move = tree.search(50);
  ASSERT_EQ(1, move);
  ASSERT_EQ(50, tree.best_found_);
  tree.apply(move);
  move = tree.search(50);
  ASSERT_EQ(2, move);
  ASSERT_EQ(50, tree.best_found_);
  tree.apply(move);
  move = tree.search(50);
  ASSERT_EQ(3, move);
  ASSERT_EQ(50, tree.best_found_);
  tree.apply(move);
  move = tree.search(50);
  ASSERT_EQ(4, move);
  ASSERT_EQ(50, tree.best_found_);
  tree.apply(move);
  move = tree.search(50);
  ASSERT_EQ(5, move);
  ASSERT_EQ(50, tree.best_found_);
  tree.apply(move);

  ASSERT_TRUE(tree.state_.is_terminal());
}
