#ifndef FACILITY_LOCATION_FLWALKER_H_
#define FACILITY_LOCATION_FLWALKER_H_

#include <vector>
#include "facility_location/util.h"

namespace facility_location
{

  /** @brief [implements Walker] */
  template<typename Instance> struct FLWalker
  {
      template<typename FacilitySet>
      FLWalker(const Instance &_instance, const FacilitySet &fs) :
        instance(_instance)
      {
        //FIXME: shall I require iterators or just operator[]?
        //FIXME: shall I put it into initialization list?
        current_set.assign(fs.begin(), fs.end());
        current_fitness_ = fitness(instance, current_set);
      }

    private:
      const Instance &instance;
      std::vector<bool> current_set, next_set;
      double current_fitness_, next_fitness_;

    public:
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
        //achievable: O(C log F)
        //browsing whole neighbourhood may be profitable
        //larger neighbourhood may be useful
        //TODO: optimize fitness calculation, assignment maintenance
        //TODO: add facility swap (apart from add/remove)
        //TODO: add some interesting step distribution
        next_set = current_set;
        size_t f = random() % next_set.size();
        next_set[f] = !next_set[f];
        f = random() % next_set.size();
        next_set[f] = !next_set[f];

        next_fitness_ = fitness(instance, next_set);
      }

      void make_step()
      {
        current_set = next_set;
        current_fitness_ = next_fitness_;
      }

  };

}  // namespace facility_location

#endif  // FACILITY_LOCATION_FLWALKER_H_
