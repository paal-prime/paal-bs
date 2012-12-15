#ifndef FACILITY_LOCATION_PRIMDUALSCHEMA_H_
#define FACILITY_LOCATION_PRIMDUALSCHEMA_H_

#include <boost/foreach.hpp>
#include <vector>
#include <algorithm>
#include <functional>
#include <queue>
#include <memory>
#include <limits>

#include "facility_location/ComposableInstance.h"

namespace facility_location {
  using std::vector;
  using std::sort;
  using std::priority_queue;
  using std::greater;

  template<typename Instance> class PrimDualSchema {
    public:
      typedef typename Instance::value_type Cost;

    private:
      typedef double time_type;
      const time_type kEpsilon = 1e-9;

      struct City;
      struct Facility {
        bool is_opened_;
        Cost to_pay_;
        Cost last_paid_;
        vector<City*> special_edges_;
        size_t payers_count_;
        Facility() : is_opened_(0), last_paid_(0), payers_count_(0) {}
        Facility(const Facility&) = delete;
        Facility & operator=(const Facility&) = delete;
      };
      struct City {
        bool is_connected_;
        vector<Facility*> special_edges_;
        City() : is_connected_(0) {}
        City(const City&) = delete;
        City & operator=(const City&) = delete;
      };
      struct EdgeEvent {
        time_type time_;
        Facility* facility_;
        City* city_;
        EdgeEvent(time_type time, Facility* facility, City* city) :
          time_(time), facility_(facility), city_(city) {
        }
      };
      struct FacilityEvent {
        time_type time_;
        Facility* facility_;
        FacilityEvent(time_type time, Facility* facility) :
          time_(time), facility_(facility) {
        }
      };
      template<typename Event, bool Greater = false> struct EventComparator {
        inline bool operator()(const Event& e1, const Event& e2) {
          if (Greater) {
            return (e1.time_ >= e2.time_);
          } else {
            return (e1.time_ < e2.time_);
          }
        }
      };
      typedef priority_queue < FacilityEvent, vector<FacilityEvent>,
              EventComparator<FacilityEvent, true> > facility_events_queue;

      // TODO(stupaq): make it const
      Instance& instance_;
      vector<EdgeEvent> edge_events_;
      facility_events_queue facility_events_;
      std::unique_ptr<Facility[]> facilities_;
      std::unique_ptr<City[]> cities_;
      time_type current_time_ = 0;
      size_t unconnected_cities_;
      // solution
      vector<size_t> assignment_;
      Cost total_cost_;

    public:
      explicit PrimDualSchema(Instance &instance) :
        instance_(instance) {
        init();
      }
      virtual ~PrimDualSchema() {
      }
      void init() {
        size_t facilities_count = instance_.facilities_count(),
               cities_count = instance_.cities_count();
        unconnected_cities_ = cities_count;
        // facilities
        facilities_.reset(new Facility[facilities_count]);
        for (size_t i = 0; i < facilities_count; i++) {
          facilities_[i].to_pay_ = instance_.opening_cost()(i);
        }
        // cities
        cities_.reset(new City[cities_count]);
        // events
        edge_events_.reserve(cities_count * facilities_count);
        for (size_t i = 0; i < facilities_count; i++) {
          for (size_t j = 0; j < cities_count; j++) {
            edge_events_.push_back(EdgeEvent(instance_.connecting_cost()(i, j),
                &facilities_[i], &cities_[j]));
          }
        }
        sort(edge_events_.begin(), edge_events_.end(),
             EventComparator<EdgeEvent, false>());
      }
      void deinit() {
        facilities_.reset();
        cities_.reset();
        edge_events_.clear();
        facility_events_ = facility_events_queue();
      }
      size_t index(Facility &facility) {
        return &facility - &facilities_[0];
      }
      size_t index(City &city) {
        return &city - &cities_[0];
      }
      bool is_zero(time_type val) {
        return fabs(val) <= kEpsilon;
      }
      void recompute_to_pay(Facility &facility) {
        facility.to_pay_ -= (current_time_ - facility.last_paid_)
            * facility.payers_count_;
        facility.last_paid_ = current_time_;
      }
      time_type recompute_expected(Facility &facility) {
        assert(facility.payers_count_ > 0);
        return (time_type) facility.to_pay_ / facility.payers_count_
               + current_time_;
      }
      void open_facility(Facility &facility) {
        if (facility.is_opened_) {
          return;
        }
        // since we have both increase and decrease key, we cannot simply
        // pass first attempt of paying
        recompute_to_pay(facility);
        if (is_zero(facility.to_pay_)) {
          BOOST_FOREACH(City * c, facility.special_edges_) {
            connect_city(facility, *c);
          }
          facility.is_opened_ = true;
        }
      }
      void add_payer_city(Facility &facility, City &city) {
        recompute_to_pay(facility);
        facility.special_edges_.push_back(&city);
        facility.payers_count_++;
        city.special_edges_.push_back(&facility);
      }
      void remove_payer_city(Facility &facility, City &city) {
        assert(facility.payers_count_ > 0);
        recompute_to_pay(facility);
        // NOTE: we do NOT have to remove cities from this list as they
        // become connected, as we cannot harm trying to connect them again
        // facility.special_edges_.remove(&city);
        if (--facility.payers_count_ > 0) {
          time_type expected_time = recompute_expected(facility);
          // TODO(stupaq): increase key instead
          facility_events_.push(FacilityEvent(expected_time, &facility));
        }
      }
      void connect_city(Facility &facility, City &city) {
        if (city.is_connected_) {
          return;
        }
        // NOTE: we use different matching algorithm, we don't need that
        // city.connection_witness_ = &facility;
        BOOST_FOREACH(Facility * f, city.special_edges_) {
          remove_payer_city(*f, city);
        }
        city.is_connected_ = true;
        --unconnected_cities_;
      }
      void handle_edge_event(const EdgeEvent& event) {
        current_time_ = event.time_;
        Facility &facility = *event.facility_;
        City &city = *event.city_;
        if (!facility.is_opened_) {
          add_payer_city(facility, city);
          time_type expected_time = recompute_expected(facility);
          // TODO(stupaq): decrease key instead
          facility_events_.push(FacilityEvent(expected_time, &facility));
        } else {
          connect_city(facility, city);
        }
      }
      void handle_facility_event(const FacilityEvent& event) {
        current_time_ = event.time_;
        open_facility(*event.facility_);
      }
      void time_simulation() {
        auto next_edge = edge_events_.begin();
        for (;;) {
          if (unconnected_cities_ == 0) {
            break;
          }
          if (next_edge != edge_events_.end()) {
            if (!facility_events_.empty()) {
              // NOTE: we give precedence to facility payment event, so that
              // edges between cities contributing to facilities are special
              if (next_edge->time_ < facility_events_.top().time_) {
                handle_edge_event(*next_edge);
                next_edge++;
              } else {
                auto event = facility_events_.top();
                facility_events_.pop();
                handle_facility_event(event);
              }
            } else {
              handle_edge_event(*next_edge);
              next_edge++;
            }
          } else if (!facility_events_.empty()) {
            auto event = facility_events_.top();
            facility_events_.pop();
            handle_facility_event(event);
          } else {
            assert(false);
          }
        }
      }
      void find_opened_facilities() {
        for (size_t i = 0; i < instance_.facilities_count(); i++) {
          Facility &facility = facilities_[i];
          if (facility.is_opened_) {
            BOOST_FOREACH(City * c, facility.special_edges_) {
              BOOST_FOREACH(Facility * f, c->special_edges_) {
                if (f != &facility) {
                  f->is_opened_ = false;
                }
              }
            }
          }
        }
      }
      void find_cities_assignment() {
        assignment_.resize(instance_.cities_count());
        for (size_t j = 0; j < instance_.cities_count(); j++) {
          Cost min_cost = std::numeric_limits<Cost>::max();
          size_t min_i = 0;
          for (size_t i = 0; i < instance_.facilities_count(); i++) {
            if (facilities_[i].is_opened_) {
              Cost c = instance_.connecting_cost()(i, j);
              if (min_cost >= c) {
                min_cost = c;
                min_i = i;
              }
            }
          }
          assert(facilities_[min_i].is_opened_);
          assignment_[j] = min_i;
        }
        total_cost_ = assignment_cost(instance_, assignment_);
      }
      Cost operator()() {
        time_simulation();
        find_opened_facilities();
        find_cities_assignment();
        deinit();
        return cost();
      }
      vector<Cost>& assignment() {
        return assignment_;
      }
      Cost cost() {
        return total_cost_;
      }
  };
}

#endif  // FACILITY_LOCATION_PRIMDUALSCHEMA_H_
