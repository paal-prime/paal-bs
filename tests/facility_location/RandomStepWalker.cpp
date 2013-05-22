#include <gtest/gtest.h>

#include <random>
#include <vector>
#include <boost/numeric/ublas/matrix.hpp>

#include "facility_location/util.h"
#include "facility_location/RandomStepWalker.h"

#include "paal/search.h"
#include "paal/ProgressCtrl.h"
#include "paal/StepCtrl.h"
#include "paal/Logger.h"

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

TEST(facility_location,RandomStepWalker_invariants)
{
	Random random(26998423);
	Instance inst; inst.gen(20,20,random);
	std::vector<bool> fs; random_facility_set(inst,fs,random);
	RandomStepWalker<Instance> walker(inst,fs);
	EXPECT_EQ(walker.current_fitness(),fitness(inst,fs));
	int D[3] = {};
	for(size_t it=0; it<1000; ++it)
	{
		walker.prepare_step(.1,random);
		std::vector<bool> set = walker.next_set;
		size_t diff = 0;
		for(size_t i=0; i< set.size(); ++i)
			diff += set[i]!=walker.current_set[i];
		EXPECT_GE(2,diff); D[diff]++;
		walker.make_step();
		EXPECT_EQ(set,walker.current_set);
		EXPECT_EQ(walker.current_fitness(),fitness(inst,set));
	}
	EXPECT_LE(300,D[1]);
	EXPECT_LE(300,D[2]);
}

// checks if RandomStepWalker implements paal::Walker concept
TEST(facility_location,RandomStepWalker_interface)
{
	Random random(26998423);
	Instance inst; inst.gen(20,20,random);
	std::vector<bool> fs; random_facility_set(inst,fs,random);
	RandomStepWalker<Instance> walker(inst,fs);
	paal::IterationCtrl progress_ctrl(1000);
	paal::HillClimb step_ctrl;
	paal::VoidLogger logger;
	paal::search(walker,random,progress_ctrl,step_ctrl,logger);
}

