#ifndef PAAL_LOGGER_H_
#define PAAL_LOGGER_H_

#include <cstdlib>

namespace paal
{
  struct VoidLogger  // implements Logger
  {
    void log(double current_fitness) {}
  };

  struct CountingLogger  // implements Logger
  {
    CountingLogger() : iterations(-1) {}
    size_t iterations;
    void log(double current_fitness)
    {
      iterations++;
    }
  };

  struct ImprovementLogger  // implements Logger
  {
	ImprovementLogger() : iterations(-1) {}
	
	struct Record
	{
		Record(size_t _iteration, double _fitness) :
			iteration(_iteration), fitness(_fitness) {}
		size_t iteration;
		double fitness;
	};
	
	size_t iterations;
	std::vector<Record> records;

	void log(double current_fitness)
	{
		if(!++iterations || records.back().fitness>current_fitness)
			records.push_back(Record(iterations,current_fitness));
	}
  };
}

#endif  // PAAL_LOGGER_H_
