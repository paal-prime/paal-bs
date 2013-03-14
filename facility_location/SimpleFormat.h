#ifndef FACILITY_LOCATION_SIMPLEFORMAT_H_
#define FACILITY_LOCATION_SIMPLEFORMAT_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <cassert>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>  // NOLINT
#include <limits>

namespace facility_location {
  // TODO(stupaq): consider switching to std::vector->ublas::Vector facade
  template<typename Value> using Matrix = boost::numeric::ublas::matrix<Value>;
  template<typename Value> using Vector = boost::numeric::ublas::vector<Value>;

  /** @brief Implementation of SimpleFormat for UflLib, fulfils instance
   * contract. */
  template<typename Cost = double> class SimpleFormat {
      const std::string kOptFileSuffix = ".opt";
    private:
      Matrix<Cost> connecting_cost_;
      Vector<Cost> opening_cost_;
      Vector<size_t> optimal_solution_;
      Cost optimal_cost_;
    public:
      typedef Cost value_type;

      /**
       * @brief constructs instance from serialized file
       * @param file path to file with instance
       **/
      SimpleFormat(const std::string &file) {
        // read instance
        std::ifstream is(file.c_str());
        if (!is) {
          throw std::runtime_error("Could not open " + file);
        }
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        size_t n, m;
        is >> n >> m;
        opening_cost_.resize(n);
        connecting_cost_.resize(n, m);
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        for (size_t i = 0; i < n; i++) {
          size_t fno;
          is >> fno >> opening_cost_(i);
          assert(fno == i + 1);
          for (size_t j = 0; j < m; j++) {
            is >> connecting_cost_(i, j);
          }
        }
        is.close();
        // read optimal solution
        const std::string file_opt = file + kOptFileSuffix;
        std::ifstream isopt(file_opt.c_str());
        if (!isopt) {
          throw std::runtime_error("Could not open " + file_opt);
        }
        optimal_solution_.resize(m);
        for (size_t j = 0; j < m; j++) {
          isopt >> optimal_solution_(j);
        }
        isopt >> optimal_cost_;
        isopt.close();
      }

      /** @returns number of cities */
      size_t cities_count() const {
        return connecting_cost_.size2();
      }

      /** @returns number of facilities */
      size_t facilities_count() const {
        return opening_cost_.size();
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

      /**
       * @brief serializes SimpleFormat instance to given stream
       * @param os output stream
       * @param instance instance to serialize
       **/
      template<typename Stream, typename CostType> friend Stream& operator<< (Stream& os, const
          SimpleFormat<CostType>& instance) {
        os << instance.connecting_cost_.size1() << ' '
           << instance.connecting_cost_.size2() << " 0 \n";
        for (int i = 0, n = instance.connecting_cost_.size1(); i < n; i++) {
          os << i + 1 << ' ' << instance.opening_cost_(i) << ' ';
          for (int j = 0, m = instance.connecting_cost_.size2(); j < m; j++) {
            os << instance.connecting_cost_(i, j) << ' ';
          }
          os << '\n';
        }
        return os;
      }
  };
}

#endif  // FACILITY_LOCATION_SIMPLEFORMAT_H_
