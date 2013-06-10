#ifndef FACILITY_LOCATION_RANDOMSTEPWALKER_H_
#define FACILITY_LOCATION_RANDOMSTEPWALKER_H_

#include <vector>
#include "facility_location/util.h"

namespace facility_location
{
  /**
   * @brief [implements Walker]
   *
   * Neighbourhood consists of all facility sets in hamming distance <= 2.
   * Prepared step is selected at random.
   **/
  template<typename Instance> struct RandomStepWalker
  {
      /**
       * @param _instance [implements Instance] problem instance
       * @param fs [implements FacilitySet] initial solution
       */
      template<typename FacilitySet>
      RandomStepWalker(const Instance &_instance, const FacilitySet &fs) :
        instance(_instance)
      {
        current_set.assign(fs.begin(), fs.end());
        current_fitness_ = fitness(instance, current_set);
      }

    private:
      const Instance &instance;
      double current_fitness_, next_fitness_;

    public:
      std::vector<bool> current_set, next_set;

      double current_fitness()
      {
        return current_fitness_;
      }
      double next_fitness()
      {
        return next_fitness_;
      }

      template<typename Random>
      void prepare_step(double progress, Random &random)
      {
        next_set = current_set;
        for (size_t times = random() & 1; times < 2; ++times)
        {
          size_t f = random() % next_set.size();
          next_set[f] = !next_set[f];
        }
        next_fitness_ = fitness(instance, next_set);
      }

      void make_step()
      {
        current_set = next_set;
        current_fitness_ = next_fitness_;
      }
  };
}  // namespace facility_location

#endif  // FACILITY_LOCATION_RANDOMSTEPWALKER_H_
