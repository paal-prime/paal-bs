#ifndef FACILITY_LOCATION_UTIL_H_
#define FACILITY_LOCATION_UTIL_H_

#include <algorithm>
#include <limits>

namespace facility_location
{
  /*
    concept FacilitySet
    {
      bool operator[](size_t i); // true iff i-th facility is opened
      void resize(size_t n); //resizes the set to n elements
      size_t size(); // number of facilities in the instance
    }
  */

  /** @brief calculates fitness of the optimal assignment
   * @param fs opened facilities that cities are being assigned to
   */
  template<typename Instance, typename FacilitySet>
  inline double fitness(const Instance &instance, const FacilitySet &fs)
  {
    double res = 0;
    for (size_t f = 0; f < instance.facilities_count(); ++f)
      if (fs[f]) res += instance(f);
    for (size_t c = 0; c < instance.cities_count(); ++c)
    {
      double best_cost = std::numeric_limits<double>::infinity();
      for (size_t f = 0; f < instance.facilities_count(); ++f) if (fs[f])
          best_cost = std::min(best_cost, instance(f, c));
      res += best_cost;
    }
    return res;
  }

  /**
   * @brief generates a random facility set
   */
  template<typename Instance, typename FacilitySet, typename Random>
  inline void random_facility_set(
    const Instance &instance, FacilitySet &fs, Random &random)
  {
    fs.resize(instance.facilities_count());
for (auto f : fs) f = random() & 1;
  }

}  // namespace facility_location

#endif  // FACILITY_LOCATION_UTIL_H_
