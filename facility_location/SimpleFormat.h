#ifndef FACILITY_LOCATION_SIMPLEFORMAT_H_
#define FACILITY_LOCATION_SIMPLEFORMAT_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <cassert>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <limits>

namespace facility_location {
  // TODO(stupaq): consider switching to std::vector->ublas::Vector facade
  template<typename Value> using Matrix = boost::numeric::ublas::matrix<Value>;
  template<typename Value> using Vector = boost::numeric::ublas::vector<Value>;

  template<typename Cost = double> class SimpleFormat {
      const std::string kOptFileSuffix = ".opt";
    private:
      Matrix<Cost> connecting_cost_;
      Vector<Cost> opening_cost_;
      Vector<Cost> optimal_solution_;
      Cost optimal_cost_;
    public:
      typedef Cost value_type;
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
      size_t cities_count() const {
        return connecting_cost_.size2();
      }
      size_t facilities_count() const {
        return opening_cost_.size();
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
      template<typename Stream> Stream& operator<< (Stream& os) const {
        os << connecting_cost_.size1() << ' '
           << connecting_cost_.size2() << " 0 \n";
        for (int i = 0, n = connecting_cost_.size1(); i < n; i++) {
          os << i + 1 << ' ' << opening_cost_(i) << ' ';
          for (int j = 0, m = connecting_cost_.size2(); j < m; j++) {
            os << connecting_cost_(i, j) << ' ';
          }
          os << '\n';
        }
        return os;
      }
  };

  template<typename Stream, typename Cost>
  Stream& operator<< (Stream& os, const SimpleFormat<Cost>& instance) {
    return instance.operator << (os);
  }
}

#endif  // FACILITY_LOCATION_SIMPLEFORMAT_H_
