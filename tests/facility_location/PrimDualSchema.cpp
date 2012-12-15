#include <gtest/gtest.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <climits>
#include <algorithm>
#include <utility>
#include <vector>

#include "facility_location/ComposableInstance.h"
#include "facility_location/PrimDualSchema.h"

using std::pair;
using std::make_pair;
using facility_location::ComposableInstance;
using facility_location::PrimDualSchema;

typedef boost::numeric::ublas::scalar_matrix<int> Matrix;
typedef boost::numeric::ublas::scalar_vector<int> Vector;
typedef ComposableInstance<int, Vector, Matrix, std::vector<int> > Instance;
typedef PrimDualSchema<int, Vector, Matrix> Solver;

class PrimDualSchemaBoundary
    : public ::testing::TestWithParam<pair<size_t, size_t> > {
  protected:
    const size_t f = 4;
    const size_t c = 3;
    Vector oc0;
    Matrix cc0;
    Vector oc1;
    Matrix cc1;
    std::vector<int> sol;

    PrimDualSchemaBoundary() : oc0(Vector(f, 0)), cc0(Matrix(f, c, 0)),
      oc1(Vector(f, 1)), cc1(Matrix(f, c, 1)),
      sol(std::vector<int>(c, 0)) {
    }
    Instance instance(pair<size_t, size_t> p, Vector& oc, Matrix& cc) {
      assert(p.first <= f && p.second <= c);
      return Instance(p.first, p.second, oc, cc, sol);
    }
};

TEST_P(PrimDualSchemaBoundary, ZeroCosts) {
  Instance i = instance(GetParam(), oc0, cc0);
  ASSERT_EQ(0, i.optimal_cost());
  Solver s(i);
  ASSERT_EQ(i.optimal_cost(), s());
}

TEST_P(PrimDualSchemaBoundary, ZeroOpeningCosts) {
  auto p = GetParam();
  Instance i = instance(p, oc0, cc1);
  ASSERT_EQ(p.second, i.optimal_cost());
  Solver s(i);
  ASSERT_EQ(i.optimal_cost(), s());
}

TEST_P(PrimDualSchemaBoundary, ZeroConnectingCosts) {
  auto p = GetParam();
  Instance i = instance(p, oc1, cc0);
  ASSERT_EQ((p.second > 0) ? 1 : 0, i.optimal_cost());
  Solver s(i);
  ASSERT_EQ(i.optimal_cost(), s());
}

INSTANTIATE_TEST_CASE_P(ZeroCities, PrimDualSchemaBoundary, ::testing::Values(
      make_pair(0, 0), make_pair(1, 0), make_pair(2, 0), make_pair(3, 0)));

INSTANTIATE_TEST_CASE_P(OneFacility, PrimDualSchemaBoundary, ::testing::Values(
      make_pair(1, 0), make_pair(1, 1), make_pair(1, 2), make_pair(1, 3)));

INSTANTIATE_TEST_CASE_P(Bigger, PrimDualSchemaBoundary, ::testing::Values(
      make_pair(4, 3), make_pair(4, 2), make_pair(3, 3), make_pair(3, 2)));
