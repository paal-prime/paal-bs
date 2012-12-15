#include <iostream>
#include <string>

#include "facility_location/SimpleFormat.h"
#include "facility_location/PrimDualSchema.h"

int main(int argc, char *argv[]) {
  using namespace facility_location;
  std::string file = argc > 1 ? argv[1] : "UflLib/Euclid/1011EuclS.txt";
  SimpleFormat<double> instance(file);
  PrimDualSchema<SimpleFormat<double> > solver(instance);
  std::cout << solver() << ' ' << instance.optimal_cost() << ' ';
  std::cout << (double) solver.cost() / instance.optimal_cost() << std::endl;
}
