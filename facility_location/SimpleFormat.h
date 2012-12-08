#ifndef FACILITY_LOCATION_SIMPLEFORMAT_H_
#define FACILITY_LOCATION_SIMPLEFORMAT_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include <limits>

namespace facility_location {
  template<typename Value> using Matrix = boost::numeric::ublas::matrix<Value>;
  template<typename Value> using Vector = boost::numeric::ublas::vector<Value>;

  const std::string kOptFileSuffix = ".opt";

  template<typename Cost = double> class SimpleFormat {
    private:
      Matrix<Cost> connecting_cost_;
      Vector<Cost> opening_cost_;
      Vector<Cost> optimal_solution_;
      Cost optimal_cost_;
    public:
      SimpleFormat(const std::string &file) {
        // read instance
        std::ifstream is(file.c_str());
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
        optimal_solution_.resize(m);
        for (size_t j = 0; j < m; j++) {
          isopt >> optimal_solution_(j);
        }
        isopt >> optimal_cost_;
        isopt.close();
      }
      size_t cities_count() {
        return connecting_cost_.size2();
      }
      size_t facilities_count() {
        return opening_cost_.size();
      }
      Matrix<Cost>& connecting_cost() {
        return connecting_cost_;
      }
      Vector<Cost>& opening_cost() {
        return opening_cost_;
      }
      Vector<Cost>& optimal_solution() {
        return optimal_solution_;
      }
      Cost optimal_cost() {
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
