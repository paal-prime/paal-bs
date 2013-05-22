#ifndef PAAL_FIDIAGRAM_H_
#define PAAL_FIDIAGRAM_H_

#include <vector>
#include <ostream>  // NOLINT
#include "format.h"
#include "paal/Logger.h"

namespace paal
{
  /** @brief [implements Table] collects data for fitness/iteration diagram */
  struct FIDiagram
  {
    double optimum;
    FIDiagram(double _optimum) : optimum(_optimum)
    {
      records.push_back(Record("optimum"));
      records.back().logger.log(1);
    }

    struct Record
    {
      explicit Record(const std::string &_name) : name(_name) {}
      std::string name;
      ImprovementLogger logger;
    };

    std::vector<Record> records;

    /**
    * @brief collects logs over single algorithm execution
    * @param name algorithm label for the Record object
    * @param algo [implements Algo] executed algorithm
    */
    template<typename Algo> void test(const std::string &name, Algo algo)
    {
      records.push_back(Record(name));
      algo.run(records.back().logger);
    }

    /** @brief prints the logs in a human readable form */
    void dump(std::ostream &os)
    {
      os << "iteration, fitness, algorithm\n";
      for (Record & r : records)
      {
        for (auto & rr : r.logger.records)
          os << format("%, %, %\n", rr.iteration, rr.fitness / optimum, r.name);
        os << format("%, %, %\n", r.logger.iterations, r.logger.records.back().fitness / optimum, r.name);
      }
    }
  };
} // namespace paal

#endif  // PAAL_FIDIAGRAM_H_
