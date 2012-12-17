#include <gtest/gtest.h>

#include "paal/StepCtrl.h"

#include <random>

TEST(paal_StepCtrl,HillClimb)
{
	paal::HillClimb step_ctrl;
	std::mt19937 random(682342);
	EXPECT_TRUE(step_ctrl.step_decision(24,2334,.3,random));
	EXPECT_FALSE(step_ctrl.step_decision(2432,233,.8,random));
}
