#ifndef MCTS_POLICY_H_
#define MCTS_POLICY_H_

#include <limits>
#include <algorithm>

namespace mcts
{

  /*
  template<typename Random> concept Policy
  {
    struct Payload
    {
      bool operator<(const Payload&) const;
    };

    Random & get_random();

    template<typename Node>
    bool expand(Node &node, size_t level);

    template<typename Node>
    size_t choose(Node &node);

    template<typename Node>
    void update(Node &node, size_t index, double fitness);
  };
  */

  /** @brief [partial mcts::Policy]
   * maintains mean estimate in the Payload
   */
  struct Mean
  {
    struct Payload
    {
      size_t visits = 0;
      Fitness estimate = std::numeric_limits<Fitness>::infinity();
      bool operator<(const Payload &b) const
      { return estimate < b.estimate; }
    };

    template<typename Node>
    void update(Node &node, ssize_t chosen, Fitness estimate)
    {
      Payload &payload = node();
      payload.visits++;
      payload.estimate = (payload.visits == 1) ? estimate :
          payload.estimate + (estimate - payload.estimate) / payload.visits;
    }
  };

  /** @brief [partial mcts::Policy]
   * expands when number of visits reaches visits_min
   */
  struct VisitsMin
  {
    size_t visits_min;
    explicit VisitsMin(size_t _visits_min) : visits_min(_visits_min) {}
    template<typename Node> bool expand(const Node &node, size_t level)
    { return node().visits >= visits_min; }
  };

  /** @brief [implements mcts::Policy]
   * Best child is the one with the best mean estimate.
   * Chooses child at random.
   */
  template<typename Random> struct PolicyRandMean : Mean, VisitsMin
  {
    private:
      Random &random_;
    public:
      explicit PolicyRandMean(Random &_random, size_t _visits_min = 20) :
        VisitsMin(_visits_min), random_(_random) {}
      Random & get_random(){ return random_; }

      template<typename Node> size_t choose(const Node &node)
      { return random_() % node.size(); }
  };

  /** @brief [implements mcts::Policy]
   * Best child is the one with the best mean estimate.
   * With probability eps chooses child at random,
   * otherwise best child until now.
   */
  template<typename Random> struct PolicyEpsMean : Mean, VisitsMin
  {
    private:
      Random &random_;
      double eps;
    public:
      explicit PolicyEpsMean(
        Random& _random, double _eps = .1, size_t _visits_min = 20) :
        VisitsMin(_visits_min), random_(_random), eps(_eps) {}

      Random & get_random(){ return random_; }

      template<typename Node> size_t choose(const Node &node)
      {
        // http://arxiv.org/pdf/1210.0374v1   page 6
        return random_() < eps * random_.max() ?
               random_() % node.size() : node.best_child();
      }
  };

  /** @brief [implements mcts::Policy]
   * Best child is the one with the best single estimate.
   * With probability eps chooses child at random,
   * otherwise best child until now.
   */
  template<typename Random> struct PolicyEpsBest : VisitsMin
  {
    private:
      Random& random_;
      double eps;
    public:
      struct Payload
      {
        size_t visits = 0;
        Fitness estimate = std::numeric_limits<Fitness>::infinity();
        size_t best_child = 0;
        Fitness best_estimate = std::numeric_limits<Fitness>::infinity();
        bool operator<(const Payload &b) const { return estimate < b.estimate; }
      };

      explicit PolicyEpsBest(
        Random &_random, double _eps = .1, size_t _visits_min = 20)
        : VisitsMin(_visits_min), random_(_random), eps(_eps) {}

      Random & get_random(){ return random_; }

      template<typename Node>
      void update(Node &node, ssize_t chosen, Fitness estimate)
      {
        Payload &p = node();
        p.visits++;
        p.estimate = std::min(p.estimate, estimate);
        if (chosen != -1 && estimate < p.best_estimate)
        {
          p.best_estimate = estimate;
          p.best_child = chosen;
        }
      }

      template<typename Node> size_t choose(const Node &node)
      {
        return (node.size() > node().best_child ||
            random_() < eps * random_.max()) ?
            random_() % node.size() : node().best_child;
      }
  };

  /** @brief [implements mcts::Policy]
   * Best child is ??
   */
  template<typename Random> struct PolicyMuSigma : VisitsMin
  {
    private:
      Random &random_;
      double discovery_factor_;

    public:
      struct Payload
      {
        size_t visits = 0;
        double mean = 0;
        double interm = 0;
        double factor = 0;
        bool operator<(const Payload &b) const
        { return factor < b.factor; }
      };

      explicit PolicyMuSigma(
        Random &_random, double discovery_factor = 1.0, size_t _visits_min = 20)
        : VisitsMin(_visits_min), random_(_random),
          discovery_factor_(discovery_factor) {}

      Random & get_random(){ return random_; }

      template<typename Node>
      void update(Node &node, ssize_t chosen, Fitness estimate)
      {
        Payload &p = node();
        p.visits++;
        double delta = estimate - p.mean;
        p.mean += delta / p.visits;
        p.interm += delta * (estimate - p.mean);
        p.factor = p.mean + (p.visits <= 1 ? 0 :
            discovery_factor_ * sqrt(p.interm / (p.visits - 1)));
      }

      template<typename Node> size_t choose(const Node &node)
      { return node.best_child(); }
  };
}  // namespace mcts

#endif  // MCTS_POLICY_H_
