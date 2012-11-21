#include <iostream>

#include "facility_location/SimpleFormat.h"
#include "facility_location/PrimDualSchema.h"

int main() {
  using namespace facility_location;
  SimpleFormat<double> instance("UflLib/Euclid/1011EuclS.txt");
  PrimDualSchema<double, Vector<double>, Matrix<double> > solver(instance);
  solver();
}
