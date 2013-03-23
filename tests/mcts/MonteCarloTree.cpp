#include <gtest/gtest.h>

#include <mcts/MonteCarloTree.h>
#include <tsp/TSPLIB.h>
#include <tsp/MCTS_tsp.h>

#include <limits>
#include <vector>
#include <utility>
#include <algorithm>

#ifdef adsf
typedef double Fitness;

typedef int TestMove;

struct TestState
{
  std::vector<TestMove> moves_ = {0, 1, 2, 3, 4, 5};

  const std::vector<TestMove> moves() const
  {
    return moves_;
  }

  void apply(const TestMove& move)
  {
    moves_.erase(std::find(moves_.begin(), moves_.end(), move));
  }

  bool is_terminal()
  {
    return moves_.empty();
  }

  Fitness estimate_playout(std::mt19937& random)
  {
    return 100;
  }
};

struct TestPolicy
{
  typedef struct
  {
    size_t visits_ = 0;
    Fitness estimate_ = std::numeric_limits<Fitness>::max();
  } Payload;

  std::mt19937 random_;

  std::mt19937& get_random()
  {
    return random_;
  }

  template<typename Node>
  void update(Node& parent, ssize_t chosen, Fitness estimate)
  {
    Payload& payload = parent();
    payload.visits_++;
    payload.estimate_ = std::min(payload.estimate_, estimate);
  }

  template<typename Node, typename State> size_t choose(const Node& parent,
      const State &state)
  {
    return 0;
  }

  template<typename Node> size_t best_child(const Node &parent)
  {
    return 0;
  }

  template<typename Node, typename State> bool expand(const Node& node,
      const State& state, size_t iteration, size_t level)
  {
    return true;
  }
};

class MonteCarloTreeTests : public testing::Test
{
};

TEST_F(MonteCarloTreeTests, Node)
{
  using mcts::Node;

  Node<TestMove, TestPolicy> node(-1);
  TestState state;
  ASSERT_GT(state.moves().size(), 2);
  node.expand(state);
  ASSERT_EQ(state.moves().size(), node.children_.size());
  for (size_t i = 0; i < state.moves().size(); i++)
  {
    ASSERT_EQ(state.moves_[i], node[i].move());
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

TEST_F(MonteCarloTreeTests, Tree)
{
  // TODO(stupaq): friend test
  using mcts::MonteCarloTree;

  TestState state;
  TestPolicy policy;
  MonteCarloTree<TestMove, TestState, TestPolicy> tree(state, policy);
  TestMove move;
  move = tree.search(50);
  ASSERT_EQ(0, move);
  tree.apply(move);
  move = tree.search(50);
  ASSERT_EQ(1, move);
  tree.apply(move);
  move = tree.search(50);
  ASSERT_EQ(2, move);
  tree.apply(move);
  move = tree.search(50);
  ASSERT_EQ(3, move);
  tree.apply(move);
  move = tree.search(50);
  ASSERT_EQ(4, move);
  tree.apply(move);
  move = tree.search(50);
  ASSERT_EQ(5, move);
  tree.apply(move);

  ASSERT_TRUE(tree.state_.is_terminal());
}
#endif
