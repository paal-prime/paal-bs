#include <gtest/gtest.h>

#include <mcts/MonteCarloTree.h>
#include <tsp/TSPLIB.h>
#include <tsp/MCTS_tsp.h>

#include <algorithm>
#include <vector>

typedef int TestMove;

struct TestState {
  std::vector<TestMove> moves_ = {0, 1, 2, 3, 4, 5};

  const std::vector<TestMove> moves() const {
    return moves_;
  }
};

struct TestPolicy {
  template<typename Node> void update(Node* node, double sample) {
    node->estimate_ = (node->visits_ == 1) ? sample : std::min(node->estimate_,
        sample);
  }
};

TEST(MonteCarloTree, Node) {
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

TEST(MonteCarloTree, Tree) {
  // TODO(stupaq)
}
