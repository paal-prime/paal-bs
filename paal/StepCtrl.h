#ifndef PAAL_STEPCTRL_H_
#define PAAL_STEPCTRL_H_

#include <cmath>
#include <cstdlib>

namespace paal
{
  /** @brief [implements StepCtrl] steps iff fitness is not worsening */
  struct HillClimb
  {
    template<typename Random> bool step_decision(double current_fitness,
        double next_fitness, double, Random &)
    {
      return current_fitness >= next_fitness;
    }
  };

  /** @brief [implements StepCtrl] steps according to annealing strategy
   *
   * see: http://en.wikipedia.org/wiki/Boltzmann_distribution
   */
  struct Annealing
  {
    Annealing(double _t0, double _t1) : t0(_t0), t1(_t1), steps(0) {}
    double t0, t1, f;
    size_t steps;
    template<typename Random> bool step_decision(double current_fitness,
        double next_fitness, double progress, Random &random)
    {
      // 100 is arbitrary; pow() is time consuming
      if (!(steps++ % 100)) f = pow(t0 / t1, progress) / t0;
      return static_cast<double>(random()) / random.max() <=
             exp((current_fitness - next_fitness) * f);
    }
  };
}  // namespace paal

#endif  // PAAL_STEPCTRL_H_
