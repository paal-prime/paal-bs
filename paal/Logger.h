#ifndef PAAL_LOGGER_H_
#define PAAL_LOGGER_H_

#include <cstdlib>

namespace paal
{
  struct VoidLogger  // implements Logger
  {
    void log(double current_fitness) {}
  };

  struct CountingLogger
  {
    CountingLogger() : iterations(-1) {}
    size_t iterations;
    void log(double current_fitness)
    {
      iterations++;
    }
  };
}

#endif  // PAAL_LOGGER_H_
