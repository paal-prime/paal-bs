#include "tsp/TSPLIB.h"
#include "tsp/TwoOptWalker.h"
#include "tsp/util.h"
#include "paal/search.h"
#include "paal/ProgressCtrl.h"
#include "paal/StepCtrl.h"
#include "paal/SuperLogger.h"
#include <random>
#include <vector>
#include <iostream>
#include "format.h"

typedef tsp::TSPLIB_Matrix Matrix;

std::mt19937 random_(786284);



struct Algo// implements Algo
{
	Algo(Matrix &_matrix) : matrix(_matrix) {}
	Matrix &matrix;

	template<typename Logger, typename StepCtrl>
	double run(Logger &logger, StepCtrl &step_ctrl) const
	{
		std::vector<size_t> cycle;
		tsp::cycle_shuffle(cycle,matrix.size1(),random_);
		tsp::TwoOptWalker<Matrix> walker(matrix,cycle);
		//paal::IterationCtrl progress_ctrl(100000);
		paal::TimeAutoCtrl progress_ctrl(5);
		double begin = paal::realtime_sec();
		paal::search(walker,random_,progress_ctrl,step_ctrl,logger);
		double end = paal::realtime_sec();
		std::cout << format("time=%\n",end-begin);
		return walker.current_fitness();
	}
};

struct HillAlgo : Algo
{
	template<typename Logger> double run(Logger &logger) const
	{
		paal::HillClimb step_ctrl;
		return run(logger,step_ctrl);
	}
};

struct AnneAlgo : Algo
{
	template<typename Logger> double run(Logger &logger) const
	{
		paal::Annealing step_ctrl(); //damnit! <- was starting point dependent until now
		return run(logger,step_ctrl);
	}
};

int main()
{
	tsp::TSPLIB_Directory dir("./TSPLIB/symmetrical/");
	Matrix matrix;
	dir.graphs[67].load(matrix);
	paal::SuperLogger sl;
	sl.test("hill_climb",HillAlgo(matrix),5);
	sl.test("annealing",AnneAlgo(matrix),5);
	sl.dump(std::cout); std::cout << std::flush;
	return 0;
}
