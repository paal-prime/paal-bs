#include <gtest/gtest.h>

#include <random>
#include <vector>
#include <boost/numeric/ublas/matrix.hpp>

#include "facility_location/util.h"
#include "facility_location/FLApxWalker.h"

using namespace facility_location;
using boost::numeric::ublas::matrix;

typedef std::mt19937 Random;

struct Instance
{
	template<typename Random>
	void gen(size_t n, size_t m, Random &random)
	{
		conn.resize(n,m);
		open.resize(n);

		for(size_t i=0; i<n; ++i)
			for(size_t j=0; j<m; ++j) conn(i,j) = random()%100;
		for(int &o : open) o = random()%100;
	}
	
	matrix<int> conn;
	std::vector<int> open;

	size_t cities_count() const
	{ return conn.size2(); }

	size_t facilities_count() const
	{ return conn.size1(); }

	double operator()(size_t facility, size_t city) const
	{ return conn(facility,city); }

	double operator()(size_t facility) const
	{ return open[facility]; }
};

TEST(FLApxWalker,Best2_random)
{
	Random random(2634723);
	Instance inst; inst.gen(6,10,random);
	std::vector<bool> fs; random_facility_set(inst,fs,random);
	for(size_t c=0; c<inst.cities_count(); ++c)
	{
		Best2 b(inst,fs,c);
		//StepCosts sc(inst,fs);
		ASSERT_TRUE(b.b1!=b.b2);
		ASSERT_TRUE(fs[b.b1] && fs[b.b2]);
		ASSERT_TRUE(b.c1==inst(b.b1,c));
		ASSERT_TRUE(b.c2==inst(b.b2,c));
		for(size_t i=0; i<fs.size(); ++i)
			for(size_t j=0; j<fs.size(); ++j)
				if(i!=j && fs[i] && fs[j] && inst(i,c)<=inst(j,c))
		{
			ASSERT_LE(b.c1,inst(i,c));
			ASSERT_LE(b.c2,inst(j,c));
		}
	}
}

TEST(FLApx_Walker,Best2_empty_set)
{
	Random random(2634723);
	Instance inst; inst.gen(6,10,random);
	std::vector<bool> fs; fs.resize(inst.facilities_count(),0);
	for(size_t c=0; c<inst.cities_count(); ++c)
	{
		Best2 b(inst,fs,c);
		ASSERT_EQ(b.b1,-1);
		ASSERT_EQ(b.b2,-1);
		ASSERT_EQ(b.c1,0);
		ASSERT_EQ(b.c2,0);
	}
}

//TODO:
TEST(FLApx_Walker,Best2_singleton)
{
	
}

TEST(FLApxWalker,StepCosts_random)
{
	double inf = std::numeric_limits<double>::infinity();
	Random random(2634723);
	Instance inst; inst.gen(20,50,random);
	std::vector<bool> fs; random_facility_set(inst,fs,random);
	StepCosts sc(inst,fs);
	for(size_t i=0; i<fs.size(); ++i) if(fs[i])
	{
		ASSERT_EQ(inf,sc.ins[i]);
		auto fs2 = fs; fs2[i] = 0;
		ASSERT_EQ(fitness(inst,fs2),sc.del[i]);
	}
	else
	{
		ASSERT_EQ(inf,sc.del[i]);
		auto fs2 = fs; fs2[i] = 1;
		ASSERT_EQ(fitness(inst,fs2),sc.ins[i]);
	}
	for(size_t i=0; i<fs.size(); ++i)
		for(size_t j=0; j<fs.size(); ++j)
			if(fs[i] || !fs[j]) ASSERT_EQ(inf,sc.cor(i,j));
	else
	{
		auto fs2 = fs; fs2[i] = 1; fs2[j] = 0;
		ASSERT_EQ(fitness(inst,fs2),sc.cor(i,j));
	}
}

TEST(FLApxWalker,StepCosts_empty_set)
{
	double inf = std::numeric_limits<double>::infinity();
	Random random(2634723);
	Instance inst; inst.gen(10,20,random);
	std::vector<bool> fs(inst.facilities_count(),0);
	StepCosts sc(inst,fs);
	for(size_t i=0; i<fs.size(); ++i)
	{
		ASSERT_EQ(inf,sc.del[i]);
		auto fs2 = fs; fs2[i] = 1;
		ASSERT_EQ(fitness(inst,fs2),sc.ins[i]);
	}
	
	for(size_t i=0; i<fs.size(); ++i)
		for(size_t j=0; j<fs.size(); ++j)
			ASSERT_EQ(inf,sc.cor(i,j));
}

TEST(FLApxWalker,StepCosts_singleton)
{
	double inf = std::numeric_limits<double>::infinity();
	Random random(2634723);
	Instance inst; inst.gen(10,20,random);
	size_t x = 3;
	std::vector<bool> fs(inst.facilities_count(),0); fs[x] = 1;
	StepCosts sc(inst,fs);
	for(size_t i=0; i<fs.size(); ++i)
	{
		ASSERT_EQ(inf,sc.del[i]);
		auto fs2 = fs; fs2[i] = 1;
		ASSERT_EQ(i==x?inf:fitness(inst,fs2),sc.ins[i]);
	}
	
	for(size_t i=0; i<fs.size(); ++i)
		for(size_t j=0; j<fs.size(); ++j)
			if(fs[i] || !fs[j]) ASSERT_EQ(inf,sc.cor(i,j));
	else
	{
		auto fs2 = fs; fs2[i] = 1; fs2[j] = 0;
		ASSERT_EQ(fitness(inst,fs2),sc.cor(i,j));
	}
}

TEST(FLApxWalker,FLApxWalker_sanity)
{

}

