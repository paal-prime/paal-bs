#include <gtest/gtest.h>
#include <cstdlib>
#include "paal/search.h"

enum
{
  cfitness = 478232,
  nfitness = 872374,
  random_val = 728342,
  random_max = 982734
};

struct MyWalker
{
  MyWalker() : steps_prepared(0), steps_made(0) {}
  size_t steps_prepared;
  size_t steps_made;

  template<typename Random> void prepare_step(double progress, Random &random)
  {
    EXPECT_GE(progress, 0);
    EXPECT_LT(progress, 1);
    EXPECT_EQ(random(), random_val);
    steps_prepared++;
  }

  void make_step()
  {
    EXPECT_LT(steps_made, steps_prepared);
    steps_made++;
  }

  double current_fitness()
  {
    return cfitness;
  }

  double next_fitness()
  {
    return nfitness;
  }
};

struct MyStepCtrl
{
  template<typename Random> bool step_decision(double current_fitness,
      double next_fitness, double progress, Random &random)
  {
    EXPECT_EQ(current_fitness, cfitness);
    EXPECT_EQ(next_fitness, nfitness);
    EXPECT_EQ(random(), random_val);
    EXPECT_GE(progress, 0);
    EXPECT_LT(progress, 1);
    return progress >= .5;
  }
};

struct MyProgressCtrl
{
  MyProgressCtrl() : iterations(0) {}
  size_t iterations;
  double progress(double current_fitness)
  {
    EXPECT_LE(iterations, 10);
    EXPECT_EQ(current_fitness, cfitness);
    return iterations++*.1;
  }
};

struct MyRandom
{
  uint32_t operator()()
  {
    return random_val;
  }
  uint32_t max()
  {
    return random_max;
  }
};

struct MyLogger
{
  MyLogger() : iterations(0) {}
  size_t iterations;
  void log(double current_fitness)
  {
    EXPECT_EQ(current_fitness, cfitness);
    iterations++;
  }
};

TEST(paal_search, sample_use)
{
  MyWalker walker;
  MyStepCtrl step_ctrl;
  MyProgressCtrl progress_ctrl;
  MyRandom random;
  MyLogger logger;
  paal::search(walker, random, progress_ctrl, step_ctrl, logger);
  ASSERT_EQ(walker.steps_prepared, 10);
  ASSERT_EQ(walker.steps_made, 5);
  ASSERT_EQ(progress_ctrl.iterations, 11);
  ASSERT_EQ(logger.iterations, 11);
}
