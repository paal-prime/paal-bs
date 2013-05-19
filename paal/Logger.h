#ifndef PAAL_LOGGER_H_
#define PAAL_LOGGER_H_

#include <cstdlib>
#include <vector>

namespace paal
{
  /** @brief [implements Logger] logger that does nothing */
  struct VoidLogger
  {
    void log(double current_fitness) {}
  };

  /** @brief [implements Logger] counts iterations passed */
  struct CountingLogger
  {
    CountingLogger() : iterations(-1) {}

    /** @brief iteration counter */
    size_t iterations;
    void log(double current_fitness)
    {
      iterations++;
    }
  };

  /** @brief [implements Logger] records fitness every iteration */
  struct IterationLogger
  {
    std::vector<double> records;
    void log(double current_fitness)
	{ records.push_back(current_fitness); }
  };

  /** @brief [implements Logger] logs fitness every time it gets lower than
   * the previous best
   */
  struct ImprovementLogger
  {
    ImprovementLogger() : iterations(-1) {}

    struct Record
    {
      Record(size_t _iteration, double _fitness) :
        iteration(_iteration), fitness(_fitness) {}
      size_t iteration;
      double fitness;
    };

    /** @brief iteration counter */
    size_t iterations;
    /** @brief recorded fitness improvements */
    std::vector<Record> records;

    void log(double current_fitness)
    {
      if (!++iterations || records.back().fitness > current_fitness)
        records.push_back(Record(iterations, current_fitness));
    }
  };
}  // namespace paal

#endif  // PAAL_LOGGER_H_
