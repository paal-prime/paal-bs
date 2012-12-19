#ifndef PAAL_STEPCTRL_H_
#define PAAL_STEPCTRL_H_

#include <cmath>
#include <cstdlib>

namespace paal
{
  struct HillClimb  // implements StepCtrl
  {
    template<typename Random> bool step_decision(double current_fitness,
        double next_fitness, double, Random &)
    {
      return current_fitness >= next_fitness;
    }
  };

  struct Annealing //implements StepCtrl
  {
    //http://pl.wikipedia.org/wiki/Rozk%C5%82ad_Boltzmanna
    Annealing(double _t0, double _t1) : t0(_t0), t1(_t1), steps(0) {}
    double t0, t1, f;
    size_t steps;
    template<typename Random> bool step_decision(double current_fitness,
        double next_fitness, double progress, Random &random)
    {
      if (!(steps++ % 100)) f = pow(t0 / t1, progress) / t0;
      return double(random()) / random.max() <=
             exp((current_fitness - next_fitness)*f);
    }
  };
}

#endif  // PAAL_STEPCTRL_H_
