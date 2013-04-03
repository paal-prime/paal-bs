#ifndef PAAL_GRIDTABLE_H_
#define PAAL_GRIDTABLE_H_

#include <vector>
#include <iomanip>
#include <ostream>

#include "paal/Logger.h"

namespace paal
{
  struct GridTable
  {
    struct Record
    {
      explicit Record(const std::string &_name) : name(_name) {}
      std::string name;
      std::vector<double> results;

      template<typename Algo> void test(Algo algo)
      {
        VoidLogger logger;
        results.push_back(algo.run(logger));
      }
    };

    std::vector<Record> records;

    void push_algo(std::string name)
    {
      records.push_back(Record(name));
    }

    void dump(std::ostream &os)
    {
      for (Record & r : records)
      {
        os << std::setw(15) << r.name;
        for (double res : r.results) os << std::setw(8) << res;
        os << '\n';
      }
      os << std::flush;
    }
  };
}

#endif  // PAAL_GRIDTABLE_H_
