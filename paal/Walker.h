#ifndef PAAL_WALKER_H_
#define PAAL_WALKER_H_

namespace paal
{
  /*
  concept Walker
  {
    / ** @return fitness of the current solution * /
    double current_fitness();
    
    / **
      * @return fitness of the solution prepared by the last prepare_step()
      * before the first prepare_step() call, the behaviour i undefined.
      * /
    double next_fitness();
    
    / **
      * @brief prepare a step to make from the current solution
      * @param progress (see ProgressCtrl)
      * @param random [implements Random]
      * /
    template<typename Random> void prepare_step(double progress, Random &random);
    
    / ** @brief step to the solution prepared by last prepare_step() * /
    void make_step();
  };
  */
}

#endif  // PAAL_WALKER_H_

