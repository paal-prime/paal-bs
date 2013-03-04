#ifndef FACILITY_LOCATION_COMPOSABLEINSTANCE_H_
#define FACILITY_LOCATION_COMPOSABLEINSTANCE_H_

namespace facility_location {

  /**
   * @brief computes cost of assignment
   * @param instance an instance of input graph
   * @param assignment an assignment
   * @returns cost of the assignment
   **/
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

  /** @brief Instance facade, it doesn't store nor take ownership of passed
   * parts of a graph. */
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
      /**
       * @brief composes instance from opening cost and connecting cost
       * @param facilities number of facilities (which are indexed from zero)
       * @param cities number of cities (which are indexed from zero)
       * @param opening called with facility index returns its opening cost
       * @param connecting called with pair (facility index, city index)
       * returns cost of connecting these vertices
       * @param optimal an optimal (or reference) assignment if available
       **/
      typedef Cost value_type;
      ComposableInstance(const size_t facilities, const size_t cities,
          const OpeningCost& opening, const ConnectingCost& connecting,
          const Assignment& optimal) : facilities_count_(facilities),
        cities_count_(cities), opening_cost_(opening),
        connecting_cost_(connecting), optimal_solution_(optimal) {
        optimal_cost_ = assignment_cost(*this, optimal);
      }

      /** @returns number of cities */
      size_t cities_count() const {
        return cities_count_;
      }

      /** @returns number of facilities */
      size_t facilities_count() const {
        return facilities_count_;
      }

      /**
       * @param facility index of a facility
       * @param city index of a city
       * @returns cost of connecting the city with the facility
       **/
      Cost operator()(size_t facility, size_t city) const {
        return connecting_cost_(facility, city);
      }

      /**
       * @param facility an index of a facility
       * @returns cost of opening the facility
       **/
      Cost operator()(size_t facility) const {
        return opening_cost_(facility);
      }

      /**
       * @param city an index of a city
       * @returns index of facility assigned to the city in optimal (reference)
       * assignment
       **/
      size_t optimal_solution(size_t city) const {
        return optimal_solution_(city);
      }

      /** @returns cost of optimal (reference) solution */
      Cost optimal_cost() const {
        return optimal_cost_;
      }
  };

  /**
   * @brief helper for creating ComposableInstance
   * @param facilities number of facilities (which are indexed from zero)
   * @param cities number of cities (which are indexed from zero)
   * @param opening called with facility index returns its opening cost
   * @param connecting called with pair (facility index, city index)
   * returns cost of connecting these vertices
   * @param optimal an optimal (or reference) assignment if available
   * @returns ComposableInstance wrapping passed instance parts
   **/
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
