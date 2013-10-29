#ifndef PAAL_PROGRESSCTRL_H_
#define PAAL_PROGRESSCTRL_H_

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <algorithm>

namespace paal
{
  /*
  concept ProgressCtrl
  {
    / **
      * @brief estimates fraction the remaining available execution time over the total available time.
      * @param current_fitness (see Walker)
      * @return value from [0;inf]
      *
      * progress()>=1 iff the time is up.
      * /
    double progress(double current_fitness);
  };
  */

  /** @brief [implements ProgressCtrl]
   * progress = iterations passed / iterations available
   */
  struct IterationCtrl
  {
    /** @param _available_it iterations available */
    explicit IterationCtrl(size_t _available_it) :
      passed_it(0), available_it(_available_it) {}
    size_t passed_it, available_it;
    double progress(double current_fitness)
    {
      return static_cast<double>(passed_it++) / available_it;
    }
  };

  /** @brief wrapper around clock_gettime, providing realtime in seconds */
  inline double realtime_sec()
  {
    timespec tv;
    assert(clock_gettime(CLOCK_REALTIME, &tv) != -1);
    return tv.tv_sec + tv.tv_nsec * 1e-9;
  }

  /** @brief [implements ProgressCtrl] progress = time passed / time available;
   * time checking granularity has to be provided
   *
   * Works well if expected execution time of each iteration is constant.
   */
  struct TimeCtrl
  {
    /**
    * @param _available_sec available time in seconds
    * @param _granularity_it number of iterations between time checkings
    */
    TimeCtrl(double _available_sec, size_t _granularity_it) :
      start_sec(realtime_sec()), available_sec(_available_sec), progress_(-1),
      passed_it(1), granularity_it(_granularity_it) {}
    double start_sec, available_sec, progress_;
    size_t passed_it, granularity_it;

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

  /** @brief [implements ProgressCtrl] progress = time passed / time available;
   * time checking granularity is adjusted automatically
   *
   * Works well if execution time of iteration doesn't change rapidly.
   */
  struct TimeAutoCtrl
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
}  // namespace paal

#endif  // PAAL_PROGRESSCTRL_H_
