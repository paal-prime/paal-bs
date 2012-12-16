#ifndef PAAL_SEARCH_H_
#define PAAL_SEARCH_H_

namespace paal
{
  template<typename Walker, typename StepCtrl, typename ProgressCtrl,
           typename Random, typename Logger>
  void search(Walker &walker, StepCtrl &step_ctrl, ProgressCtrl &progress_ctrl,
              Random &random, Logger &logger)
  {
    while (1)
    {
      double current_fitness = walker.current_fitness();
      logger.log(current_fitness);
      double progress = progress_ctrl.progress(current_fitness);
      if (progress >= 1) break;
      walker.prepare_step(progress, random);
      if (step_ctrl.step_decision(current_fitness, walker.next_fitness(),
          progress, random)) walker.make_step();
    }
  }
}

#endif  // PAAL_SEARCH_H_
