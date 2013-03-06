// Copyright 2013 <przed_deadlinem_zdazymy>
#include <boost/program_options.hpp>

#include <sys/time.h>
#include <random>
#include <string>
#include <iostream>
#include <vector>
#include <set>

#include "tsp/annealing.h"
#include "tsp/Christofides.h"
#include "tsp/CycleWalker.h"
#include "tsp/monitor.h"
#include "tsp/TSPLIB.h"

#include "./format.h"

using std::vector;
namespace po = boost::program_options;

static const int CHRIST_FLAG = 0x0001;
static const int ANNEAL_FLAG = 0x0002;
static const int HILL_FLAG = 0x0004;

/*
 * @brief testing routine for different TSP heuristics.
 */
int main(int argc, char **argv) {
    po::options_description desc("Allowed options");
    desc.add_options()
                ("help", "produce help message")
                ("path", po::value<std::string>(), "path to TSPLIB")
                ("tests", po::value<std::vector<int> >()->multitoken(),
                  "tests to run (OR them to combine): \n"
                  "1 - Christofides, \n"
                  "2 - annealing, \n"
                  "4 - hill climb \n"
                  "default: 1 3 5 2 4")
                ("time_limit", po::value<int>()->default_value(30),
                  "time limit for meta heuristics in seconds")
                ("cases", po::value<std::vector<std::string> >()->multitoken(),
                  "name of cases to run separated by spaces \n"
                  "empty means all \n"
                  "default: empty");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help") || !vm.count("path")) {
        std::cout << desc << std::endl;
        return 1;
    }

    std::vector<int> tests;
    if (vm.count("tests"))
        tests = vm["tests"].as<vector<int> >();
    else
        tests = {CHRIST_FLAG, CHRIST_FLAG | ANNEAL_FLAG,
                 CHRIST_FLAG | HILL_FLAG, ANNEAL_FLAG, HILL_FLAG};

    std::set<std::string> cases;
    if (vm.count("cases")) {
      std::vector<std::string> cases_param =
          vm["cases"].as<std::vector<std::string> >();
      cases.insert(cases_param.begin(), cases_param.end());
    }

    tsp::TSPLIB_Directory dir(vm["path"].as<std::string>());
    tsp::TSPLIB_Matrix mtx;
    timespec start, end;
    double real_time;
    std::string method;
    for (auto const &graph_pair : dir.graphs) {
        if (!cases.empty() && !cases.count(graph_pair.first)) continue;
        auto &graph = graph_pair.second;
        std::cout << std::endl << graph_pair.first << std::endl
                << format("optimal fitness= %", graph.optimal_fitness)
                << std::endl;
        for (int flag : tests) {
            method = "";
            std::string ewt = graph.load(mtx);

            typedef std::mt19937 Random;
            Random random(64236738);
            double time = vm["time_limit"].as<int>();

            tsp::DynCycle cycle;

            clock_gettime(CLOCK_REALTIME, &start);

            cycle.resize(mtx.size1());

            if (flag & CHRIST_FLAG) {
              method += "Christofides";
              if (ewt == "EUC_2D" || ewt == "CEIL_2D" || ewt == "ATT") {
                  tsp::christofides<tsp::TSPLIB_Matrix, tsp::DynCycle, double>
                  (mtx, cycle, mtx.size1(), ewt, mtx.X.get(), mtx.Y.get());
              } else {
                  tsp::christofides<tsp::TSPLIB_Matrix, tsp::DynCycle>
                  (mtx, cycle, mtx.size1());
              }
            } else {
              tsp::cycle_shuffle(cycle, mtx.size1(), random);
            }

            tsp::CycleWalker<tsp::TSPLIB_Matrix, Random>
            walker(mtx, cycle, random);
            tsp::TimeMonitor monitor(time);
            tsp::BoltzmannDistr<Random>
            distr(tsp::fitness(mtx, cycle) / mtx.size1(), 1e-300, random);

            if (flag & HILL_FLAG) {
                method += "+ hill_climb";
                tsp::hill_climb(walker, monitor);
            }

            if (flag & ANNEAL_FLAG) {
                method += "+ annealing";
                tsp::annealing(walker, monitor, distr);
            }

            clock_gettime(CLOCK_REALTIME, &end);
            real_time = (end.tv_sec - start.tv_sec) * 1000 +
                    (end.tv_nsec - start.tv_nsec) / 1000000;
            std::cout << format("%: fitness=% (%) steps=% real_time=%",
                                method, walker.best_fitness,
                                walker.best_fitness / graph.optimal_fitness,
                                monitor.steps, real_time)
                      << std::endl;
        }
    }
    return 0;
}

