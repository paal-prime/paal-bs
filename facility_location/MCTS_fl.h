#ifndef FACILITY_LOCATION_MCTS_FL_H_
#define FACILITY_LOCATION_MCTS_FL_H_

#include <mcts/MonteCarloTree.h>

#include <random>
#include <limits>
#include <cmath>
#include <algorithm>
#include <vector>
#include <numeric>
#include <functional>
#include <utility>

namespace facility_location
{
  using mcts::Fitness;


  /**
   * @brief Class representing a state in solving
   *        facility location problem instance.
   *
   * It's designed to work in incremental way by opening facilities
   * one by one.
   *
   * @tparam Matrix Class that should have two operators:
   *       (i) - returing cost of opening i-th facility.
   *       (i, j) - returning cost of connecting i-th facility with j-th city.
   */
  template<typename Matrix> class FLState
  {
    private:
      const Matrix& matrix_;
      const double facility_cost_;
      const size_t facilities_count_;
      const size_t cities_count_;
      //! Order in which cities/facilities are being processed.
      std::vector<size_t> ordering_;
      //! Already open facilities.
      std::vector<size_t> facilities_;

      /**
       * @brief calculates current cost of assignment.
       */
      Fitness calculate_cost()
      {
        Fitness cal_cost(0);
        std::vector<Fitness> costs(cities_count_,
                                   std::numeric_limits<Fitness>::infinity());
        for (auto facility : facilities_)
        {
          for (size_t i = 0; i < cities_count_; ++i)
            costs[i] = std::min(costs[i], matrix_(facility, i));
          cal_cost += matrix_(facility);
        }
        for (size_t i = 0; i < cities_count_; ++i)
          cal_cost += costs[i];
        return cal_cost;
      }

      /**
       * @brief calculates distance of given city to the closest open facility
       */
      double distance_from_facilities(size_t city)
      {
        double dist = std::numeric_limits<double>::infinity();
        for (auto facility : facilities_)
        {
          dist = std::min(dist, matrix_(facility, city));
        }
        return dist;
      }


    public:
      typedef bool Move;

      explicit FLState(const Matrix& matrix, double facility_cost,
                       size_t facilities_count, size_t cities_count) :
          matrix_(matrix), facility_cost_(facility_cost),
          facilities_count_(facilities_count), cities_count_(cities_count),
          ordering_(facilities_count)
      {
        for (size_t i = 0; i < facilities_count_; ++i)
        {
          ordering_[i] = i;
        }
        std::random_shuffle(ordering_.begin(), ordering_.end());
      }

      Fitness cost() { return calculate_cost(); }

      /**
       * @brief State is terminal iff no more cities are left to process.
       */
      bool is_terminal() const
      {
        return ordering_.size() == 0;
      }

      /**
       * @brief Applies a move to the state.
       */
      void apply(const Move& move)
      {
        if (move) facilities_.push_back(ordering_.back());
        ordering_.pop_back();
      }

      /**
       * @brief Estimates rest of playout by using Meyerson's
       *        incremental algorithm.
       */
      template<typename Random> Fitness estimate_playout(Random& random)
      {
        double rand, dist;
        while (ordering_.size())
        {
          rand = static_cast<double>(random())/random.max();
          dist = distance_from_facilities(ordering_.back());
          if (dist/facility_cost_ > rand)
            facilities_.push_back(ordering_.back());
          ordering_.pop_back();
        }
        return calculate_cost();
      }

      /**
       * @brief Basically returns only two possible moves,
       *        either create facility at given point or not.
       */
      const std::vector<Move> moves() const
      {
        return {Move(false), Move(true)};
      }

      size_t moves_count() const { return ordering_.size() ? 2 : 0; }

      size_t left_decisions() const { return ordering_.size(); }
  };
}
#endif  // FACILITY_LOCATION_MCTS_FL_H_
