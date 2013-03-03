#ifndef PAAL_SUPERLOGGER_H_
#define PAAL_SUPERLOGGER_H_

/**
  * @file SuperLogger.h
  * @brief loggers used to build final result diagrams for algorithm comparisons
  */

#include <cstdlib>
#include <ostream>  // NOLINT
#include <string>
#include <vector>
#include "paal/Logger.h"
#include "./format.h"

namespace paal
{
  /*
  concept Algo
  {
    template<typename Logger> double run(Logger &logger) // runs copy of algorithm instance with the logger given
  };
  */

  /**
   * @brief logs fitness and number of iterations of the algorithm
   * for a number of executions
   */
  struct SuperLogger
  {
    /** @brief internal representation of a single algorithm run */
	struct Result
    {
      Result(double _fitness, size_t _iterations) :
          fitness(_fitness), iterations(_iterations) {}
      double fitness;
      size_t iterations;
    };

    /** @brief internal representation of the algorithm */
	struct Record
    {
      explicit Record(const std::string &_name) : name(_name) {}
      std::string name;
      std::vector<Result> results;
    };

    /**
	 * @brief runs the algorithms and collects the logs
	 * @param name algorithm name used to label the Record structure
	 * @param algo [implements Algo] algorithm to be run
	 * @param times number of algorithm executions to perform
	 */
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

	/** @brief prints the logs in a human readable form */
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

  /** @brief logger keeping track of the algorithm's fitness improvements
   * over iterations.
   */
  struct ImprovementSuperLogger
  {
    /** @brief internal representation of a single algorithm run */
  	struct Record
	{
		Record(const std::string &_name) : name(_name) {}
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
		for(Record &r : records)
		{
			for(auto &rr : r.logger.records)
				os << format("%, %, %\n",r.name,rr.iteration,rr.fitness);
		}
	}
  };
}

#endif  // PAAL_SUPERLOGGER_H_

