#include <gtest/gtest.h>
#include <boost/test/output_test_stream.hpp>

#include <string>
#include <sstream>
#include <fstream>
#include <limits>

#include "facility_location/SimpleFormat.h"
#include "facility_location/PrimDualSchema.h"

using std::string;
using std::stringstream;
using std::istream;
using std::ifstream;
using boost::test_tools::output_test_stream;
using facility_location::SimpleFormat;
using facility_location::PrimDualSchema;

class SimpleFormatInstance
    : public ::testing::TestWithParam<const char*> {
  protected:
    const std::string kUflLibDir = "./UflLib/";

    SimpleFormat<double> instance(const string &file) {
      return SimpleFormat<double>(file);
    }
    bool compare_streams(istream &is1, stringstream &is2) {
      std::istreambuf_iterator<char> eos;
      std::string s(std::istreambuf_iterator<char>(is1), eos);
      return s == is2.str();
    }
};

TEST_P(SimpleFormatInstance, ReadWrite) {
  string file(kUflLibDir + GetParam());
  auto i = instance(file);
  ifstream is(file.c_str());
  ASSERT_FALSE(!is);
  is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  stringstream iss;
  iss << i;
  ASSERT_TRUE(compare_streams(is, iss));
}

TEST(SimpleFormatInstanceMisc, FailTest) {
  const char kNonExisting[] = "./UflLib/non_existing_file.txt";
  ASSERT_ANY_THROW(SimpleFormat<double> i(kNonExisting));
}

static const char* ufwlib_euklid[] = {"Euclid/1011EuclS.txt",
    "Euclid/1111EuclS.txt",    "Euclid/111EuclS.txt", "Euclid/1211EuclS.txt",
    "Euclid/1311EuclS.txt",    "Euclid/1411EuclS.txt", "Euclid/1511EuclS.txt",
    "Euclid/1611EuclS.txt",    "Euclid/1711EuclS.txt", "Euclid/1811EuclS.txt",
    "Euclid/1911EuclS.txt",    "Euclid/2011EuclS.txt", "Euclid/2111EuclS.txt",
    "Euclid/211EuclS.txt",    "Euclid/2211EuclS.txt", "Euclid/2311EuclS.txt",
    "Euclid/2411EuclS.txt",    "Euclid/2511EuclS.txt", "Euclid/2611EuclS.txt",
    "Euclid/2711EuclS.txt",    "Euclid/2811EuclS.txt", "Euclid/2911EuclS.txt",
    "Euclid/3011EuclS.txt",    "Euclid/311EuclS.txt", "Euclid/411EuclS.txt",
    "Euclid/511EuclS.txt",    "Euclid/611EuclS.txt", "Euclid/711EuclS.txt",
    "Euclid/811EuclS.txt",    "Euclid/911EuclS.txt"};

INSTANTIATE_TEST_CASE_P(Euclid, SimpleFormatInstance,
    ::testing::ValuesIn(ufwlib_euklid));
