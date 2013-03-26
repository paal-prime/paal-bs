#include <gtest/gtest.h>

#include <mcts/MonteCarloTree.h>
#include <tsp/TSPLIB.h>
#include <tsp/MCTS_tsp.h>
#include <paal/ProgressCtrl.h>

#include <limits>
#include <vector>
#include <utility>
#include <algorithm>

using mcts::Fitness;

struct TestState
{
  typedef int Move;

  std::vector<Move> moves_ = {0, 1, 2, 3, 4, 5};

  const std::vector<Move> moves() const { return moves_; }

  void apply(const Move& move)
  {
    moves_.erase(std::find(moves_.begin(), moves_.end(), move));
  }

  bool is_terminal() { return moves_.empty(); }

  Fitness estimate_playout(std::mt19937& random) { return 100; }
};

struct TestPolicy
{
  typedef struct
  {
    size_t visits_ = 0;
    Fitness estimate_ = std::numeric_limits<Fitness>::infinity();
  } Payload;

  std::mt19937 random_;

  std::mt19937& get_random() { return random_; }

  template<typename Node>
  void update(Node& parent, ssize_t chosen, Fitness estimate)
  {
    Payload& payload = parent();
    payload.visits_++;
    payload.estimate_ = std::min(payload.estimate_, estimate);
  }

  template<typename Node, typename State> size_t choose(const Node& parent,
      const State &state) { return 0; }

  template<typename Node> size_t best_child(const Node &parent)
  {
    assert(!parent.is_leaf());
    Fitness best = std::numeric_limits<Fitness>::infinity();
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
      const State& state, size_t iteration, size_t level) { return true; }
};

class MonteCarloTreeTests : public testing::Test {};

TEST_F(MonteCarloTreeTests, Node)
{
  typedef mcts::MonteCarloTree<TestState, TestPolicy>::Node node_type;

  node_type node(-1);
  TestState state;
  ASSERT_GT(state.moves().size(), 2);
  node.expand(state);
  ASSERT_EQ(state.moves().size(), node.size());
  for (size_t i = 0; i < state.moves().size(); i++)
  {
    ASSERT_EQ(state.moves_[i], node[i].move());
  }
  TestPolicy policy;

  node_type &chld0 = node[0];
  policy.update(chld0, -1, 100);
  ASSERT_EQ(100, chld0().estimate_);
  policy.update(node, 0, 100);
  ASSERT_EQ(0, policy.best_child(node));
  ASSERT_EQ(100, node().estimate_);

  node_type &chld2 = node[2];
  policy.update(chld2, -1, 50);
  ASSERT_EQ(50, chld2().estimate_);
  policy.update(node, 2, 50);
  ASSERT_EQ(2, policy.best_child(node));
  ASSERT_EQ(50, node().estimate_);
}

TEST_F(MonteCarloTreeTests, Tree)
{
  using mcts::MonteCarloTree;

  TestState state;
  TestPolicy policy;
  MonteCarloTree<TestState, TestPolicy> tree(state, policy);
  typename TestState::Move move;
  for (size_t i = 0; i < 6; i++)
  {
    paal::IterationCtrl ctrl(50);
    move = tree.search(ctrl);
    ASSERT_EQ(i, move);
    tree.apply(move);
  }

  ASSERT_TRUE(tree.root_state().is_terminal());
}
