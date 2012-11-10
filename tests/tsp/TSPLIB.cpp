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

TEST(TSPLIB_Directory,dist)
{
	//canonical cycle lengths for symmetrical tsp problems' sample 
	//http://www.iwr.uni-heidelberg.de/groups/comopt/software/TSPLIB95/DOC.PS
	std::string path = "./TSPLIB/symmetrical";
	const char *filenames[] = { "/pcb442.tsp", "/gr666.tsp", "/att532.tsp" };
	int fitnesses[] = { 221440, 423710, 309636 };
	tsp::TSPLIB_Directory dir(path);
	tsp::TSPLIB_Matrix mtx;
	for(auto &g : dir.graphs) for(int i=0; i<3; ++i)
		if(path + filenames[i] == g.filename)
	{
		g.load(mtx);
		int fitness = 0;
		for(size_t j=0; j<mtx.size1(); ++j)
			fitness += mtx(j,(j+1)%mtx.size1());
		EXPECT_EQ(fitness,fitnesses[i]);
	}
}

TEST(TSPLIB,triange_inequality)
// check triangle inequality in TSPLIB explicit matrices
{
	std::string path = "./TSPLIB/symmetrical";
	tsp::TSPLIB_Directory dir(path);
	tsp::TSPLIB_Matrix mtx;
	for(auto &g : dir.graphs)
	{
		g.load(mtx);
		if(mtx.dist_) continue;
		bool ok = 1;
		for(size_t i=0; i<mtx.size1(); ++i)
			for(size_t j=0; j<mtx.size1(); ++j)
				for(size_t k=0; k<mtx.size1(); ++k)
					if(mtx(i,j)>mtx(i,k)+mtx(k,j))
					{ i = j = k = mtx.size1(); ok = 0; }
		std::cout << format("% %",g.filename,ok?"OK":"INVALID") << std::endl;
	}
}
