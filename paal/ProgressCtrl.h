#ifndef PAAL_PROGRESSCTRL_H_
#define PAAL_PROGRESSCTRL_H_

#include <cassert>
#include <cstdlib>
#include <ctime>

namespace paal
{
  struct IterationCtrl  // implements ProgressCtrl
  {
    IterationCtrl(size_t _available_it) :
        passed_it(0), available_it(_available_it) {}
    size_t passed_it, available_it;
    double progress(double)
    {
      return double(passed_it++) / available_it;
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
        available_sec(_available_sec), progress_(-1),
        passed_it(1), granularity_it(_granularity_it)
    {
      start_sec = realtime_sec();
    }
    double start_sec, available_sec, progress_;
    size_t passed_it, granularity_it;

    // assumes that expected execution time of each iteration is constant
    double progress(double)
    {
      if (!--passed_it)
      {
        if (progress_ < 0)
        {
          start_sec = realtime_sec();
          progress_ = 0;
        }
        else progress_ = (realtime_sec() - start_sec) / available_sec;
        passed_it = granularity_it;
      }
      return progress_;
    }
  };

  struct TimeAutoCtrl  // implements ProgressCtrl
  {
    TimeAutoCtrl(double _available_sec) :
        available_sec(_available_sec), progress_(0),
        passed_it(0), granularity_it(0)
    {
      start_sec = realtime_sec();
    }

    double start_sec, checkpoint_sec, available_sec, progress_;
    size_t passed_it, granularity_it;

    double progress(double)
    {
      if (passed_it++ == granularity_it)
      {
        if (!granularity_it)
        {
          checkpoint_sec = realtime_sec();
          granularity_it = 1;
          passed_it = 0;
          progress_ = 0;
        }
        else
        {
          double now_sec = realtime_sec();
          granularity_it /= 100 * (now_sec - checkpoint_sec);  // aim at .01s step
          checkpoint_sec = now_sec;
          passed_it = 0;
          progress_ = (checkpoint_sec - start_sec) / available_sec;
        }
      }
      return progress_;
    }
  };

}

#endif  // PAAL_PROGRESSCTRL_H_
