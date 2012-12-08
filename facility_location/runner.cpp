#include <iostream>

#include "facility_location/SimpleFormat.h"

int main() {
  using namespace facility_location;
  SimpleFormat<> instance("UflLib/Euclid/1011EuclS.txt");
  std::cout << instance.optimal_cost() << '\n';
}
