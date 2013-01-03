#include <gtest/gtest.h>

#include <cstdlib>

#include "paal/ProgressCtrl.h"

TEST(paal_ProgressCtrl, IterationCtrl)
{
  enum { it = 10, fitness = 123 };
  paal::IterationCtrl progress_ctrl(it);
  for (size_t i = 0; i <= it; ++i)
    ASSERT_DOUBLE_EQ(static_cast<double>(i) / it,
        progress_ctrl.progress(fitness));
}

TEST(paal_ProgressCtrl, TimeCtrl_sanity)
{
  enum { gr = 3, fitness = 82934 };
  paal::TimeCtrl progress_ctrl(.5, gr);
  double prev = 0;
  for (size_t i = 0; i < 10*gr; ++i)
  {
    double prog = progress_ctrl.progress(fitness);
    if (i % gr) ASSERT_EQ(prev, prog);
    else
    {
      ASSERT_LE(prev, prog);
      ASSERT_LE(prog, .5);
    }
    prev = prog;
  }
}

TEST(paal_ProgressCtrl, TimeAutoCtrl_sanity)
{
  enum { fitness = 67823 };
  paal::TimeAutoCtrl progress_ctrl(.5);
  double prev = 0;
  for (size_t i = 0; i < 10; ++i)
  {
    double prog = progress_ctrl.progress(fitness);
    ASSERT_LE(prev, prog);
    ASSERT_LE(prog, .5);
    prev = prog;
  }
}
