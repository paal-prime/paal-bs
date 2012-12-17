#include <gtest/gtest.h>

#include "paal/ProgressCtrl.h"

#include <cstdlib>

TEST(paal_StepCtrl,IterationCtrl)
{
	enum { it = 10, fitness = 123 };
	paal::IterationCtrl progress_ctrl(it);
	for(size_t i=0; i<=it; ++i)
		ASSERT_DOUBLE_EQ(double(i)/it,progress_ctrl.progress(fitness));
}

TEST(paal_StepCtrl,TimeCtrl)
{
	enum { gr = 3, fitness = 82934 };
	paal::TimeCtrl progress_ctrl(.5,gr);
	double prev = 0;
	for(size_t i=0; i<10*gr; ++i)
	{
		double prog = progress_ctrl.progress(fitness);
		if(i%gr) ASSERT_EQ(prev,prog);
		else
		{
			ASSERT_LE(prev,prog);
			ASSERT_LE(prog,.5);
		}
		prev = prog;
	}
}
