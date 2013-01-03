#ifndef PAAL_PROGRESSCTRL_H_
#define PAAL_PROGRESSCTRL_H_

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <algorithm>

namespace paal
{
  struct IterationCtrl  // implements ProgressCtrl
  {
    explicit IterationCtrl(size_t _available_it) :
        passed_it(0), available_it(_available_it) {}
    size_t passed_it, available_it;
    double progress(double current_fitness)
    {
      return static_cast<double>(passed_it++) / available_it;
    }
  };

  inline double realtime_sec()
  {
    timespec tv;
    assert(clock_gettime(CLOCK_REALTIME, &tv) != -1);
    return tv.tv_sec + tv.tv_nsec*1e-9;
  }

  struct TimeCtrl  // implements ProgressCtrl
  {
    TimeCtrl(double _available_sec, size_t _granularity_it) :
        start_sec(realtime_sec()), available_sec(_available_sec), progress_(-1),
        passed_it(1), granularity_it(_granularity_it) {}
    double start_sec, available_sec, progress_;
    size_t passed_it, granularity_it;

    // assumes that expected execution time of each iteration is constant
    double progress(double current_fitness)
    {
      if (!--passed_it)
      {
        passed_it = granularity_it;
        progress_ = (realtime_sec() - start_sec) / available_sec;
      }
      return progress_;  // TODO(pompon): add linear interpolation if needed
    }
  };

  struct TimeAutoCtrl  // implements ProgressCtrl
  {
    explicit TimeAutoCtrl(double _available_sec) :
        start_sec(realtime_sec()), checkpoint_sec(start_sec),
        available_sec(_available_sec), progress_(0), passed_it(1),
        granularity_it(0), checkpoint_count(0) {}

    double start_sec, checkpoint_sec, available_sec, progress_;
    size_t passed_it, granularity_it, checkpoint_count;

    double progress(double current_fitness)
    {
      if (!--passed_it)
      {
        checkpoint_count++;  // value useful only for debugging
        double now_sec = realtime_sec();
        // aim at .01s step
        granularity_it /= std::max(.5, 100 * (now_sec - checkpoint_sec));
        if (!granularity_it) granularity_it = 1;
        checkpoint_sec = now_sec;
        passed_it = granularity_it;
        progress_ = (now_sec - start_sec) / available_sec;
      }
      return progress_;  // TODO(pompon): add linear interpolation if needed
    }
  };
}

#endif  // PAAL_PROGRESSCTRL_H_
