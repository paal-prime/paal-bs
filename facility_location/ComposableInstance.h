#ifndef FACILITY_LOCATION_COMPOSABLEINSTANCE_H_
#define FACILITY_LOCATION_COMPOSABLEINSTANCE_H_

namespace facility_location {
  template<typename Instance, typename Assignment>
  static inline typename Instance::value_type assignment_cost(
    Instance& instance, const Assignment& assignment) {
    bool opened[instance.facilities_count()];
    for (size_t i = 0; i < instance.facilities_count(); i++) {
      opened[i] = false;
    }
    typename Instance::value_type total = 0;
    for (size_t city = 0; city < instance.cities_count(); city++) {
      size_t facility = assignment[city];
      total += instance.connecting_cost()(facility, city);
      if (!opened[facility]) {
        total += instance.opening_cost()(facility);
        opened[facility] = true;
      }
    }
    return total;
  }

  template<typename OpeningCost, typename ConnectingCost, typename Assignment>
  class ComposableInstance {
      typedef typename OpeningCost::value_type Cost;

    private:
      size_t facilities_count_;
      size_t cities_count_;
      OpeningCost& opening_cost_;
      ConnectingCost& connecting_cost_;
      Assignment& optimal_solution_;
      Cost optimal_cost_;

    public:
      typedef Cost value_type;
      ComposableInstance(size_t facilities, size_t cities,
          OpeningCost& opening, ConnectingCost& connecting,
          Assignment& optimal) : facilities_count_(facilities),
        cities_count_(cities), opening_cost_(opening),
        connecting_cost_(connecting), optimal_solution_(optimal) {
        optimal_cost_ = assignment_cost(*this, optimal);
      }
      size_t cities_count() {
        return cities_count_;
      }
      size_t facilities_count() {
        return facilities_count_;
      }
      ConnectingCost& connecting_cost() {
        return connecting_cost_;
      }
      OpeningCost& opening_cost() {
        return opening_cost_;
      }
      Assignment& optimal_solution() {
        return optimal_solution_;
      }
      Cost optimal_cost() {
        return optimal_cost_;
      }
  };

  template<typename OpeningCost, typename ConnectingCost, typename Assignment>
  static inline ComposableInstance<OpeningCost, ConnectingCost, Assignment>
  make_instance(size_t facilities, size_t cities, OpeningCost& opening,
      ConnectingCost& connecting, Assignment& optimal) {
    return ComposableInstance<OpeningCost, ConnectingCost, Assignment>
           (facilities, cities, opening, connecting, optimal);
  }
}

#endif  // FACILITY_LOCATION_COMPOSABLEINSTANCE_H_
