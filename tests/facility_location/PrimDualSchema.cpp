#include <gtest/gtest.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <climits>
#include <algorithm>
#include <utility>
#include <vector>

#include "facility_location/ComposableInstance.h"
#include "facility_location/SimpleFormat.h"
#include "facility_location/PrimDualSchema.h"

using std::pair;
using std::make_pair;
using boost::numeric::ublas::vector;
using boost::numeric::ublas::matrix;
using facility_location::ComposableInstance;
using facility_location::SimpleFormat;
using facility_location::PrimDualSchema;
using facility_location::make_instance;

typedef boost::numeric::ublas::matrix<double> Matrix;
typedef boost::numeric::ublas::vector<double> Vector;
typedef boost::numeric::ublas::scalar_matrix<double> SMatrix;
typedef boost::numeric::ublas::scalar_vector<double> SVector;
typedef std::vector<size_t> Assignment;
typedef ComposableInstance<Vector, Matrix, Assignment> Instance;
typedef PrimDualSchema<Instance> Solver;
typedef ComposableInstance<SVector, SMatrix, Assignment> SInstance;
typedef PrimDualSchema<SInstance> SSolver;

class PrimDualSchemaMisc : public ::testing::Test {
  protected:
    const size_t f = 4;
    const size_t c = 3;
    SVector oc0;
    SMatrix cc1;
    Assignment sol;

    PrimDualSchemaMisc() : oc0(SVector(f, 0)), cc1(SMatrix(f, c, 1)),
      sol(Assignment(c, 0)) {
    }
};

TEST_F(PrimDualSchemaMisc, lazyFunctor) {
  SInstance i = make_instance(f, c, oc0, cc1, sol);
  ASSERT_EQ(c, i.optimal_cost());
  SSolver s(i);
  ASSERT_EQ(i.optimal_cost(), s());
  ASSERT_EQ(i.optimal_cost(), s());
  ASSERT_EQ(i.optimal_cost(), s());
}

class PrimDualSchemaBoundary
    : public ::testing::TestWithParam<pair<size_t, size_t> > {
  protected:
    const size_t f = 4;
    const size_t c = 3;
    SVector oc0;
    SMatrix cc0;
    SVector oc1;
    SMatrix cc1;
    Assignment sol;

    PrimDualSchemaBoundary() : oc0(SVector(f, 0)), cc0(SMatrix(f, c, 0)),
      oc1(SVector(f, 1)), cc1(SMatrix(f, c, 1)),
      sol(Assignment(c, 0)) {
    }
    SInstance instance(pair<size_t, size_t> p, SVector& oc, SMatrix& cc) {
      assert(p.first <= f && p.second <= c);
      return make_instance(p.first, p.second, oc, cc, sol);
    }
};

TEST_P(PrimDualSchemaBoundary, ZeroCosts) {
  SInstance i = instance(GetParam(), oc0, cc0);
  ASSERT_EQ(0, i.optimal_cost());
  SSolver s(i);
  ASSERT_EQ(i.optimal_cost(), s());
}

TEST_P(PrimDualSchemaBoundary, ZeroOpeningCosts) {
  auto p = GetParam();
  SInstance i = instance(p, oc0, cc1);
  ASSERT_EQ(p.second, i.optimal_cost());
  SSolver s(i);
  ASSERT_EQ(i.optimal_cost(), s());
}

TEST_P(PrimDualSchemaBoundary, ZeroConnectingCosts) {
  auto p = GetParam();
  SInstance i = instance(p, oc1, cc0);
  ASSERT_EQ((p.second > 0) ? 1 : 0, i.optimal_cost());
  SSolver s(i);
  ASSERT_EQ(i.optimal_cost(), s());
}

INSTANTIATE_TEST_CASE_P(ZeroCities, PrimDualSchemaBoundary, ::testing::Values(
      make_pair(0, 0), make_pair(1, 0), make_pair(2, 0), make_pair(3, 0)));

INSTANTIATE_TEST_CASE_P(OneFacility, PrimDualSchemaBoundary, ::testing::Values(
      make_pair(1, 0), make_pair(1, 1), make_pair(1, 2), make_pair(1, 3)));

INSTANTIATE_TEST_CASE_P(Bigger, PrimDualSchemaBoundary, ::testing::Values(
      make_pair(4, 3), make_pair(4, 2), make_pair(3, 3), make_pair(3, 2)));

class PrimDualSchemaHard
    : public ::testing::TestWithParam<size_t> {
  protected:
    // NOTE: this eps is data-size dependent
    const double kEpsilon = 1e-11;
    const size_t f = 2;
    const size_t max_c = 10000;
    Vector oc;
    Matrix cc;
    Assignment sol;

    PrimDualSchemaHard() : oc(Vector(f)), cc(Matrix(f, max_c)),
      sol(Assignment(max_c, 1)) {
      oc(0) = kEpsilon;
      oc(1) = (max_c + 1) * kEpsilon;
      cc(0, 0) = 1;
      cc(1, 0) = 1;
      for (size_t i = 1; i < max_c; i++) {
        cc(0, i) = 3;
        cc(1, i) = 1;
      }
    }
    Instance instance(size_t c) {
      assert(c <= max_c);
      oc(1) = (c + 1) * kEpsilon;
      return make_instance(f, c, oc, cc, sol);
    }
    double expected_optimal(size_t c) {
      return (double) 3 * (c - 1) + 1 + kEpsilon;
    }
};

TEST_P(PrimDualSchemaHard, ApxRatioTight) {
  size_t c = GetParam();
  auto i = instance(c);
  EXPECT_GE(expected_optimal(c) + kEpsilon, i.optimal_cost());
  Solver s(i);
  double ratio = s() / i.optimal_cost();
  EXPECT_LE(1.0, ratio);
  ASSERT_GE(3.0, ratio);
}

INSTANTIATE_TEST_CASE_P(PrimeSeries, PrimDualSchemaHard,
    ::testing::Values(2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47,
        53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127,
        131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197));

INSTANTIATE_TEST_CASE_P(Exp2Series, PrimDualSchemaHard,
    ::testing::Values(1, 2, 4, 8, 16, 32, 64, 128, 256));

INSTANTIATE_TEST_CASE_P(Exp3Series, PrimDualSchemaHard,
    ::testing::Values(1, 3, 9, 27, 81, 243));

INSTANTIATE_TEST_CASE_P(Exp10Series, PrimDualSchemaHard,
    ::testing::Values(1, 100, 1000, 10000));

class PrimDualSchemaUflLib
    : public ::testing::TestWithParam<const char*> {
  protected:
    const std::string kUflLibDir = "./UflLib/";
};

TEST_P(PrimDualSchemaUflLib, ApxRatio3) {
  std::string file(GetParam());
  SimpleFormat<double> i(kUflLibDir + file);
  PrimDualSchema<SimpleFormat<double> > s(i);
  double ratio = s() / i.optimal_cost();
  EXPECT_LE(1.0, ratio);
  ASSERT_GE(3.0, ratio);
}

const char* ufwlib_euklid[] = {"Euclid/1011EuclS.txt", "Euclid/1111EuclS.txt",
    "Euclid/111EuclS.txt", "Euclid/1211EuclS.txt", "Euclid/1311EuclS.txt",
    "Euclid/1411EuclS.txt", "Euclid/1511EuclS.txt", "Euclid/1611EuclS.txt",
    "Euclid/1711EuclS.txt", "Euclid/1811EuclS.txt", "Euclid/1911EuclS.txt",
    "Euclid/2011EuclS.txt", "Euclid/2111EuclS.txt", "Euclid/211EuclS.txt",
    "Euclid/2211EuclS.txt", "Euclid/2311EuclS.txt", "Euclid/2411EuclS.txt",
    "Euclid/2511EuclS.txt", "Euclid/2611EuclS.txt", "Euclid/2711EuclS.txt",
    "Euclid/2811EuclS.txt", "Euclid/2911EuclS.txt", "Euclid/3011EuclS.txt",
    "Euclid/311EuclS.txt", "Euclid/411EuclS.txt", "Euclid/511EuclS.txt",
    "Euclid/611EuclS.txt", "Euclid/711EuclS.txt", "Euclid/811EuclS.txt",
    "Euclid/911EuclS.txt"
                              };
INSTANTIATE_TEST_CASE_P(Euclid, PrimDualSchemaUflLib,
    ::testing::ValuesIn(ufwlib_euklid));
