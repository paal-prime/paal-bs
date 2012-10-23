//Grzegorz Prusak
#include <sstream>
#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

#include "tsp/util.h"
#include "tsp/LazyCycle.h"

using namespace tsp;

typedef tsp::LazyCycle<int> LC;

template<typename Cycle> void identity(Cycle &cycle, size_t n)
{
	cycle.resize(n);
	for(size_t i=0; i<n; ++i) cycle[i] = i;
}

template<typename Array> void assert_cycle(const LC &cycle, const Array &A)
{
	for(size_t i=0; i<cycle.size(); ++i) if(cycle[i]!=A[i])
	{
		std::stringstream ss;
		ss << '\n'; for(size_t j=0; j<cycle.size(); ++j) ss << cycle[j] << ' ';
		ss << '\n'; for(size_t j=0; j<cycle.size(); ++j) ss << A[j] << ' ';
		throw std::runtime_error(ss.str());
	}
}

TEST(LazyCycle,constructor)
{
	LC cycle;
	EXPECT_FALSE(cycle.size());
}

TEST(LazyCycle,operator_assign)
{
	LC c1,c2;
	identity(c1,10);
	c2 = c1;
	assert_cycle(c1,c2);	
}

TEST(LazyCycle,resize)
{
	size_t n = 10;
	LC cycle;
	identity(cycle,n);
	EXPECT_EQ(cycle.size(),n);
	for(size_t i=0; i<n; ++i) EXPECT_EQ(cycle[i],int(i));
	for(size_t i=0; i<1000; ++i)
	{
		cycle.resize(i);
		EXPECT_EQ(cycle.size(),i);
	}
}

TEST(LazyCycle,reverse)
{
	LC cycle;
	identity(cycle,4);
	cycle.reverse(1,3);
	int A1[] = { 0,2,1,3 };
	assert_cycle(cycle,A1);

	identity(cycle,10);
	cycle.reverse(2,6);
	int A2[] = { 0, 1, 5, 4, 3, 2, 6, 7, 8, 9 };
	assert_cycle(cycle,A2);
	cycle.reverse(0,5);
	int A3[] = { 3, 4, 5, 1, 0, 2, 6, 7, 8, 9 };
	assert_cycle(cycle,A3);
	cycle.reverse(8,8);
	assert_cycle(cycle,A3);
	cycle.reverse(0,10);
	int A4[] = { 9, 8, 7, 6, 2, 0, 1, 5, 4, 3 };
	assert_cycle(cycle,A4);
}

TEST(LazyCycle,random)
{
	srand(623746283);
	size_t n = 100;
	LC c1;
	std::vector<int> c2;
	identity(c1,n);
	identity(c2,n);
	for(size_t i=0; i<1000; ++i)
	{
		size_t l = rand()%n, r = rand()%(n-l+1)+l;
		c1.reverse(l,r);
		cycle_reverse(c2,l,r);
		assert_cycle(c1,&c2[0]);
	}
}

