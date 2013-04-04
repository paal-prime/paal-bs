#ifndef MCTS_POLICIES_H_
#define MCTS_POLICIES_H_

#include <limits>
#include <algorithm>

namespace mcts {

  template<typename Node, typename Funct>
  ssize_t min_child(const Node &parent, Funct fun) {
    Fitness best = std::numeric_limits<Fitness>::infinity();
    ssize_t index = -1;
    for (size_t i = 0; i < parent.size(); i++)
    {
      Fitness estimate = fun(parent[i]());
      if (best > estimate) { best = estimate; index = i; }
    }
    return index;
  }

  template<typename Random = std::mt19937> class PolicyRandMean
  {
    private:
      Random& random_;

    public:
      typedef struct
      {
        size_t visits_ = 0;
        Fitness estimate_ = std::numeric_limits<Fitness>::infinity();
      } Payload;

      explicit PolicyRandMean(Random& random) : random_(random) {}

      std::mt19937& get_random() { return random_; }

      template<typename Node>
      void update(Node& parent, ssize_t chosen, Fitness estimate)
      {
        Payload& payload = parent();
        payload.visits_++;
        payload.estimate_ = (payload.visits_ == 1) ? estimate :
          payload.estimate_ + (estimate - payload.estimate_) / payload.visits_;
      }

      template<typename Node, typename State> size_t choose(const Node& parent,
          const State &state)
      {
        return (random_() % parent.size());
      }

      template<typename Node> size_t best_child(const Node &parent)
      {
        ssize_t index = min_child(parent, [](const Payload& payload)
            -> double { return payload.estimate_; });
        return static_cast<size_t>(index);
      }

      template<typename Node, typename State> bool expand(const Node& node,
          const State& state, size_t iteration, size_t level)
      {
        return node().visits_ >= state.left_decisions();
      }
  };

  template<typename Random = std::mt19937> class PolicyRandEpsMean
  {
    private:
      Random& random_;
      size_t best_pick_period_;

    public:
      typedef struct
      {
        size_t visits_ = 0;
        Fitness estimate_ = std::numeric_limits<Fitness>::infinity();
      } Payload;

      explicit PolicyRandEpsMean(Random& random,
          size_t best_pick_period = 10)
        : random_(random), best_pick_period_(best_pick_period) {}

      std::mt19937& get_random() { return random_; }

      template<typename Node>
      void update(Node& parent, ssize_t chosen, Fitness estimate)
      {
        Payload& payload = parent();
        payload.visits_++;
        payload.estimate_ = (payload.visits_ == 1) ? estimate :
          payload.estimate_ + (estimate - payload.estimate_) / payload.visits_;
      }

      template<typename Node, typename State> size_t choose(const Node& parent,
          const State &state)
      {
        return (random_() % best_pick_period_ == 0)
          ? std::min(best_child(parent), parent.size() - 1)
          : (random_() % parent.size());
      }

      template<typename Node> size_t best_child(const Node &parent)
      {
        ssize_t index = min_child(parent, [](const Payload& payload)
            -> double { return payload.estimate_; });
        return static_cast<size_t>(index);
      }

      template<typename Node, typename State> bool expand(const Node& node,
          const State& state, size_t iteration, size_t level)
      {
        return node().visits_ >= state.left_decisions();
      }
  };

  template<typename Random = std::mt19937> class PolicyRandEpsBest
  {
    private:
      Random& random_;
      size_t best_pick_period_;

    public:
      typedef struct
      {
        size_t visits_ = 0;
        Fitness estimate_ = std::numeric_limits<Fitness>::infinity();
        size_t best_child_ = 0;
        Fitness best_estimate_ = std::numeric_limits<Fitness>::infinity();
      } Payload;

      explicit PolicyRandEpsBest(Random& random,
          size_t best_pick_period = 10)
        : random_(random), best_pick_period_(best_pick_period) {}

      std::mt19937& get_random() { return random_; }

      template<typename Node>
      void update(Node& parent, ssize_t chosen, Fitness estimate)
      {
        Payload& payload = parent();
        payload.visits_++;
        payload.estimate_ = std::min(payload.estimate_, estimate);
        if (chosen != (ssize_t) (-1) && payload.best_estimate_ > estimate) {
          payload.best_estimate_ = estimate;
          payload.best_child_ = chosen;
        }
      }

      template<typename Node, typename State> size_t choose(const Node& parent,
          const State &state)
      {
        return (parent().best_child_ < parent.size()
            && random_() % best_pick_period_ == 0)
          ? parent().best_child_
          : (random_() % parent.size());
      }

      template<typename Node> size_t best_child(const Node &parent)
      {
        return parent().best_child_;
      }

      template<typename Node, typename State> bool expand(const Node& node,
          const State& state, size_t iteration, size_t level)
      {
        return node().visits_ >= state.left_decisions();
      }
  };

  template<typename Random = std::mt19937> class PolicyMuSigma
  {
    private:
      Random& random_;
      size_t samples_count_;
      double discovery_factor_;

    public:
      typedef struct
      {
        size_t visits_ = 0;
        double mean_ = 0;
        double interm_ = 0;
      } Payload;

      explicit PolicyMuSigma(Random& random, size_t samples_count = 10,
          double discovery_factor = 1.0)
        : random_(random), samples_count_(samples_count),
        discovery_factor_(discovery_factor) {}

      std::mt19937& get_random() { return random_; }

      template<typename Node>
      void update(Node& parent, ssize_t chosen, Fitness estimate)
      {
        Payload &payload = parent();
        payload.visits_++;
        double delta = estimate - payload.mean_;
        payload.mean_ += delta / payload.visits_;
        payload.interm_ += delta * (estimate - payload.mean_);
      }

      template<typename Node, typename State> size_t choose(const Node& parent,
          const State &state)
      { return best_child(parent); }

      template<typename Node> size_t best_child(const Node &parent)
      {
        double factor = discovery_factor_;
        ssize_t index = min_child(parent, [&factor]
            (const Payload& payload) -> double {
            return payload.mean_ + ((payload.visits_ <= 1) ? 0
              : factor * sqrt(payload.interm_ / (payload.visits_ - 1)));
            });
        assert(index >= 0);
        return static_cast<size_t>(index);
      }

      template<typename Node, typename State> bool expand(const Node& node,
          const State& state, size_t iteration, size_t level)
      {
        return node().visits_ >= state.left_decisions();
      }
  };
}

#endif  // MCTS_POLICIES_H_
