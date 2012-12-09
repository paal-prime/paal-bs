#ifndef FACILITY_LOCATION_PRIMDUALSCHEMA_H_
#define FACILITY_LOCATION_PRIMDUALSCHEMA_H_

#include <boost/foreach.hpp>
#include <vector>
#include <algorithm>
#include <functional>
#include <utility>
#include <queue>
#include <memory>
#include <limits>

namespace facility_location {
  using std::vector;
  using std::pair;
  using std::make_pair;
  using std::sort;
  using std::priority_queue;
  using std::greater;

  // TODO(stupaq): in case of equal times, default comparator compares also
  // other positions in tuple, we do not need that
  template<typename Cost, typename OpeningCost, typename ConnectingCost>
  class PrimDualSchema {
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
      const double kEpsilon = 1e-9;

    private:
      const size_t cities_count_;
      const size_t facilities_count_;
      const ConnectingCost &connecting_cost_;
      const OpeningCost &opening_cost_;
      vector<pair<Cost, pair<Facility*, City*> > > edge_events_;
      priority_queue < pair<Cost, Facility*>, vector<pair<Cost, Facility*> >,
                     greater<pair<Cost, Facility*> > > facility_events_;
      std::unique_ptr<Facility[]> facilities_;
      std::unique_ptr<City[]> cities_;
      Cost current_time_ = 0;
      size_t unconnected_cities_;
      // solution
      vector<size_t> assignment_;
      Cost total_cost_;

    public:
      template<typename Instance> explicit PrimDualSchema(Instance &instance) :
        cities_count_(instance.cities_count()),
        facilities_count_(instance.facilities_count()),
        connecting_cost_(instance.connecting_cost()),
        opening_cost_(instance.opening_cost()) {
        init();
      }
      PrimDualSchema(
        size_t cities_count,
        size_t facilities_count,
        const OpeningCost &opening_cost,
        const ConnectingCost &connecting_cost) :
        cities_count_(cities_count), facilities_count_(facilities_count),
        connecting_cost_(connecting_cost), opening_cost_(opening_cost) {
        init();
      }
      virtual ~PrimDualSchema() {
      }
      void init() {
        unconnected_cities_ = cities_count_;
        // facilities
        facilities_.reset(new Facility[facilities_count_]);
        for (size_t i = 0; i < facilities_count_; i++) {
          facilities_[i].to_pay_ = opening_cost_(i);
        }
        // cities
        cities_.reset(new City[cities_count_]);
        // events
        edge_events_.reserve(cities_count_ * facilities_count_);
        for (size_t i = 0; i < facilities_count_; i++) {
          for (size_t j = 0; j < cities_count_; j++) {
            edge_events_.push_back(make_pair(connecting_cost_(i, j),
                make_pair(&facilities_[i], &cities_[j])));
          }
        }
        sort(edge_events_.begin(), edge_events_.end());
      }
      void deinit() {
        facilities_.reset();
        cities_.reset();
        edge_events_.clear();
        // facility_events_.clear();
      }
      size_t index(Facility &facility) {
        return &facility - &facilities_[0];
      }
      size_t index(City &city) {
        return &city - &cities_[0];
      }
      bool is_zero(Cost val) {
        return fabs(val) <= kEpsilon;
      }
      void recompute_to_pay(Facility &facility) {
        facility.to_pay_ -= (current_time_ - facility.last_paid_)
            * facility.payers_count_;
        facility.last_paid_ = current_time_;
      }
      Cost recompute_expected(Facility &facility) {
        assert(facility.payers_count_ > 0);
        return (Cost) facility.to_pay_ / facility.payers_count_
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
          Cost expected_time = recompute_expected(facility);
          // TODO(stupaq): increase key instead
          facility_events_.push(make_pair(expected_time, &facility));
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
      void handle_edge_event(const pair<Cost, pair<Facility*, City*> > event) {
        current_time_ = event.first;
        Facility &facility = *event.second.first;
        City &city = *event.second.second;
        if (!facility.is_opened_) {
          add_payer_city(facility, city);
          Cost expected_time = recompute_expected(facility);
          // TODO(stupaq): decrease key instead
          facility_events_.push(make_pair(expected_time, &facility));
        } else {
          connect_city(facility, city);
        }
      }
      void handle_facility_event(const pair<Cost, Facility*> event) {
        current_time_ = event.first;
        open_facility(*event.second);
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
              if (next_edge->first < facility_events_.top().first) {
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
        for (size_t i = 0; i < facilities_count_; i++) {
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
        assignment_.resize(cities_count_);
        total_cost_ = 0;
        for (size_t j = 0; j < cities_count_; j++) {
          Cost min_cost = std::numeric_limits<Cost>::max();
          size_t min_i = 0;
          for (size_t i = 0; i < facilities_count_; i++) {
            if (facilities_[i].is_opened_) {
              Cost c = connecting_cost_(i, j);
              if (min_cost >= c) {
                min_cost = c;
                min_i = i;
              }
            }
          }
          assert(facilities_[min_i].is_opened_);
          assignment_[j] = min_i;
          total_cost_ += min_cost;
        }
        for (size_t i = 0; i < facilities_count_; i++) {
          facilities_[i].is_opened_ = 0;
        }
        for (size_t j = 0; j < cities_count_; j++) {
          size_t i = assignment_[j];
          if (!facilities_[i].is_opened_) {
            total_cost_ += opening_cost_(i);
            facilities_[i].is_opened_ = true;
          }
        }
      }
      Cost operator()() {
        // TODO(stupaq): corner cases n == 0 || m == 0
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
