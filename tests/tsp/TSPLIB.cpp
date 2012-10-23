//Grzegorz Prusak
#include <gtest/gtest.h>

#include "tsp/TSPLIB.h"

TEST(TSPLIB_Matrix,constructor)
{
	tsp::TSPLIB_Matrix m;
	EXPECT_EQ(m.size1(),0);
	EXPECT_FALSE(m.X.get());
	EXPECT_FALSE(m.Y.get());
	EXPECT_EQ(m.mtx.size1(),0);
}

TEST(TSPLIB_Matrix,resize)
{
	tsp::TSPLIB_Matrix m;
	m.resize(10,10);
	EXPECT_EQ(m.size1(),10);
	EXPECT_FALSE(m.X.get());
	EXPECT_FALSE(m.Y.get());
	EXPECT_EQ(m.mtx.size1(),10);
	m.resize(20,20,m.eucl_dist);
	EXPECT_EQ(m.size1(),20);
	EXPECT_TRUE(m.X.get());
	EXPECT_TRUE(m.Y.get());
	EXPECT_EQ(m.mtx.size1(),0);
}
