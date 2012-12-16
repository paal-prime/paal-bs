#ifndef FACILITY_LOCATION_COMPOSABLEINSTANCE_H_
#define FACILITY_LOCATION_COMPOSABLEINSTANCE_H_

namespace facility_location {
  template<typename Instance, typename Assignment>
  static inline typename Instance::value_type assignment_cost(
    const Instance& instance, const Assignment& assignment) {
    bool opened[instance.facilities_count()];
    for (size_t i = 0; i < instance.facilities_count(); i++) {
      opened[i] = false;
    }
    typename Instance::value_type total = 0;
    for (size_t city = 0; city < instance.cities_count(); city++) {
      size_t facility = assignment(city);
      total += instance(facility, city);
      if (!opened[facility]) {
        total += instance(facility);
        opened[facility] = true;
      }
    }
    return total;
  }

  template<typename OpeningCost, typename ConnectingCost, typename Assignment>
  class ComposableInstance {
      typedef typename OpeningCost::value_type Cost;

    private:
      const size_t facilities_count_;
      const size_t cities_count_;
      const OpeningCost& opening_cost_;
      const ConnectingCost& connecting_cost_;
      const Assignment& optimal_solution_;
      Cost optimal_cost_;

    public:
      typedef Cost value_type;
      ComposableInstance(const size_t facilities, const size_t cities,
          const OpeningCost& opening, const ConnectingCost& connecting,
          const Assignment& optimal) : facilities_count_(facilities),
        cities_count_(cities), opening_cost_(opening),
        connecting_cost_(connecting), optimal_solution_(optimal) {
        optimal_cost_ = assignment_cost(*this, optimal);
      }
      size_t cities_count() const {
        return cities_count_;
      }
      size_t facilities_count() const {
        return facilities_count_;
      }
      Cost operator()(size_t facility, size_t city) const {
        return connecting_cost_(facility, city);
      }
      Cost operator()(size_t facility) const {
        return opening_cost_(facility);
      }
      size_t optimal_solution(size_t city) const {
        return optimal_solution_(city);
      }
      Cost optimal_cost() const {
        return optimal_cost_;
      }
  };

  template<typename OpeningCost, typename ConnectingCost, typename Assignment>
  static inline ComposableInstance<OpeningCost, ConnectingCost, Assignment>
  make_instance(const size_t facilities, const size_t cities,
      const OpeningCost& opening, const ConnectingCost& connecting,
      const Assignment& optimal) {
    return ComposableInstance<OpeningCost, ConnectingCost, Assignment>
           (facilities, cities, opening, connecting, optimal);
  }
}

#endif  // FACILITY_LOCATION_COMPOSABLEINSTANCE_H_
