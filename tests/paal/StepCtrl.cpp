#include <random>

#include <gtest/gtest.h>

#include "paal/StepCtrl.h"

TEST(paal_StepCtrl, HillClimb)
{
  paal::HillClimb step_ctrl;
  std::mt19937 random(682342);
  EXPECT_TRUE(step_ctrl.step_decision(2334, 24, .3, random));
  EXPECT_FALSE(step_ctrl.step_decision(233, 2432, .8, random));
}

TEST(paal_StepCtrl, Annealing_sanity)
{
  paal::Annealing step_ctrl(1, 1e-6);
  std::mt19937 random(982732);
  for (size_t i = 0; i < 50; ++i)
    EXPECT_TRUE(step_ctrl.step_decision(623, 611, .4, random));
  for (size_t i = 0; i < 50; ++i)
    step_ctrl.step_decision(random() % 1000, random() % 1000, i * .02, random);
}
