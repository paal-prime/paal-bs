#ifndef FACILITY_LOCATION_COMPOSABLEINSTANCE_H_
#define FACILITY_LOCATION_COMPOSABLEINSTANCE_H_

namespace facility_location {
  template < typename Cost, typename OpeningCost, typename ConnectingCost,
           typename Assignment >
  static inline Cost assignment_cost(size_t facilities_count, size_t cities_count,
      const OpeningCost& opening_cost, const ConnectingCost& connecting_cost,
      const Assignment& assignment) {
    bool opened[facilities_count];
    for (size_t i = 0; i < facilities_count; i++) {
      opened[i] = false;
    }
    Cost total = 0;
    for (size_t city = 0; city < cities_count; city++) {
      size_t facility = assignment[city];
      total += connecting_cost(facility, city);
      if (!opened[facility]) {
        total += opening_cost(facility);
        opened[facility] = true;
      }
    }
    return total;
  }

  template < typename Cost, typename OpeningCost, typename ConnectingCost,
           typename Assignment >
  class ComposableInstance {
    private:
      size_t facilities_count_;
      size_t cities_count_;
      OpeningCost& opening_cost_;
      ConnectingCost& connecting_cost_;
      Assignment& optimal_solution_;
      Cost optimal_cost_;

    public:
      ComposableInstance(size_t facilities, size_t cities,
          OpeningCost& opening, ConnectingCost& connecting,
          Assignment& optimal) : facilities_count_(facilities),
        cities_count_(cities), opening_cost_(opening),
        connecting_cost_(connecting), optimal_solution_(optimal) {
        optimal_cost_ = assignment_cost<Cost>(facilities, cities,
            opening, connecting, optimal);
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
}

#endif  // FACILITY_LOCATION_COMPOSABLEINSTANCE_H_
