#include <random>
#include <vector>
#include <iostream>
#include <fstream>

#include "tsp/TSPLIB.h"
#include "tsp/TwoOptWalker.h"
#include "tsp/util.h"

#include "paal/search.h"
#include "paal/ProgressCtrl.h"
#include "paal/StepCtrl.h"
#include "paal/FIDiagram.h"

#include "format.h"
#include "result_dir.h"

typedef tsp::TSPLIB_Matrix Matrix;

std::mt19937 random_(9823429);


struct Algo  // implements Algo
{
  Algo(Matrix &_matrix, size_t _it) : matrix(_matrix), it(_it) {}
  Matrix &matrix;
  size_t it; //iterations

  template<typename Logger, typename StepCtrl, typename Cycle>
  double run_(Logger &logger, StepCtrl &step_ctrl, const Cycle &cycle) const
  {
    tsp::TwoOptWalker<Matrix> walker(matrix, cycle);
    paal::IterationCtrl progress_ctrl(it);
    //paal::TimeAutoCtrl progress_ctrl(15);
    //double begin = paal::realtime_sec();
    paal::search(walker, random_, progress_ctrl, step_ctrl, logger);
	//std::cout << "final cost: " << tsp::fitness(matrix,walker.cycle) << " " << walker.current_fitness() << std::endl;
    //double end = paal::realtime_sec();
    //std::cout << format("time=%\n",end-begin);
    return walker.current_fitness();
  }
};

struct HillAlgo : Algo
{
  HillAlgo(Matrix &_matrix, size_t _it) : Algo(_matrix,_it) {}

  template<typename Logger> double run(Logger &logger) const
  {
    std::vector<size_t> cycle;
    tsp::cycle_shuffle(cycle, matrix.size1(), random_);
	//std::cout << "initial cost: " << tsp::fitness(matrix,cycle) << std::endl;
    paal::HillClimb step_ctrl;
    return run_(logger, step_ctrl, cycle);
  }
};

struct AnneAlgo : Algo
{
  AnneAlgo(Matrix &_matrix, size_t _it, double _t1) :
	  Algo(_matrix,_it), t1(_t1) {}
  double t1;

  template<typename Logger> double run(Logger &logger) const
  {
    std::vector<size_t> cycle;
    tsp::cycle_shuffle(cycle, matrix.size1(), random_);
    paal::Annealing step_ctrl(
      tsp::fitness(matrix, cycle) / matrix.size1(), t1);
    return run_(logger, step_ctrl, cycle);
  }
};

int main(int argc, char **argv)
{
  Dir out_dir(argc,argv);
  tsp::TSPLIB_Directory dir("./TSPLIB/symmetrical/");
  std::vector<std::string> graph_ids =
  {
	  "brazil58",
	  "d15112",
	  "ulysses22",
  };
  std::vector<size_t> it =
  {
	100000,
	1000000,
	10000,
  };

  Matrix matrix;
  for(size_t i=0; i<graph_ids.size(); ++i)
  {
  	paal::FIDiagram dia(0);
    dir.graphs[graph_ids[i]].load(matrix);
    dia.test("hill climb", HillAlgo(matrix,it[i]));
    dia.test("annealing 1e-9", AnneAlgo(matrix,it[i],1e-9));
    dia.test("annealing 1e-5", AnneAlgo(matrix,it[i],1e-5));
    dia.test("annealing 1e-2", AnneAlgo(matrix,it[i],1e-2));
	std::ofstream f(out_dir(graph_ids[i]+".tex"));
	dia.reduce(.01);
    dia.dump_tex(f,graph_ids[i]);
	std::cout << graph_ids[i] << " done" << std::endl;
  }
  return 0;
}
