#ifndef PAAL_SUPERLOGGER_H_
#define PAAL_SUPERLOGGER_H_

#include <cstdlib>
#include <ostream>  // NOLINT
#include <string>
#include <vector>
#include "paal/Logger.h"
#include "./format.h"

namespace paal
{
  struct SuperLogger
  {
    struct Result
    {
      Result(double _fitness, size_t _iterations) :
          fitness(_fitness), iterations(_iterations) {}
      double fitness;
      size_t iterations;
    };

    struct Record
    {
      explicit Record(const std::string &_name) : name(_name) {}
      std::string name;
      std::vector<Result> results;
    };

    template<typename Algo> void test(
      const std::string &name, Algo algo, size_t times)
    {
      data.push_back(Record(name));
      while (times--)
      {
        CountingLogger logger;
        double fitness = algo.run(logger);
        data.back().results.push_back(Result(fitness, logger.iterations));
      }
    }

    std::vector<Record> data;

    void dump(std::ostream &os)
    {
      for (Record &r : data)
      {
        os << r.name;
        for (Result &res : r.results)
          os << format(", %", res.fitness);
        os << '\n';
      }
    }
  };
}

#endif  // PAAL_SUPERLOGGER_H_

