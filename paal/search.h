#ifndef PAAL_SEARCH_H_
#define PAAL_SEARCH_H_

namespace paal
{
  template < typename Walker, typename Random, typename ProgressCtrl,
  typename StepCtrl, typename Logger >
  void search(Walker &walker, Random &random, ProgressCtrl &progress_ctrl,
      StepCtrl &step_ctrl, Logger &logger)
  {
    progress_ctrl.reset();
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
  
  /*template < typename Walker, typename Random, typename ProgressCtrl,
  typename StepCtrl >
  void search(Walker &walker, Random &random, ProgressCtrl &progress_ctrl,
      StepCtrl &step_ctrl)
  {
    VoidLogger logger;
    search(walker, random, progress_ctrl, step_ctrl, logger);
  }

  template<typename Walker, typename Random, typename ProgressCtrl>
  void search(Walker &walker, Random &random, ProgressCtrl &progress_ctrl)
  {
    HillClimb step_ctrl;
    search(walker, random, progress_ctrl, step_ctrl);
  }*/

}

#endif  // PAAL_SEARCH_H_
