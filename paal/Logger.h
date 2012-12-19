#ifndef PAAL_LOGGER_H_
#define PAAL_LOGGER_H_

#include <cstdlib>

namespace paal
{
  struct VoidLogger  // implements Logger
  {
    void log(double) {}
  };

  struct CountingLogger
  {
  	CountingLogger() : iterations(-1) {}
	size_t iterations;
	void log(double){ iterations++; }
  };
}

#endif  // PAAL_LOGGER_H_
