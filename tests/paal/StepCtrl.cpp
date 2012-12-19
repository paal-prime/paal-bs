#include <gtest/gtest.h>

#include "paal/StepCtrl.h"

#include <random>

TEST(paal_StepCtrl,HillClimb)
{
	paal::HillClimb step_ctrl;
	std::mt19937 random(682342);
	EXPECT_TRUE(step_ctrl.step_decision(2334,24,.3,random));
	EXPECT_FALSE(step_ctrl.step_decision(233,2432,.8,random));
}
