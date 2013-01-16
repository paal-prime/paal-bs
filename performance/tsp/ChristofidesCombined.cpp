// Copyright 2013 <przed_deadlinem_zdarzymy>
#include <boost/program_options.hpp>

#include <sys/time.h>
#include <random>
#include <string>
#include <iostream>
#include <vector>

#include "tsp/annealing.h"
#include "tsp/Christofides.h"
#include "tsp/CycleWalker.h"
#include "tsp/monitor.h"
#include "tsp/TSPLIB.h"

#include "./format.h"

namespace po = boost::program_options;

static const int ANNEAL_FLAG = 0x0001;
static const int HILL_FLAG = 0x0002;

/*
 * It's a testing routine for different TSP heuristics.
 */
int main(int argc, char **argv) {
    po::options_description desc("Allowed options");
    desc.add_options()
                ("help", "produce help message")
                ("path", po::value<std::string>(), "path to TSPLIB")
                ("tests", po::value<std::vector<int> >(),
                        "tests to run: \n"
                        "0 - only Christofides, \n"
                        "1 - with annealing, \n"
                        "2 - with hill climb \n"
                        "default: all of them")
                ("time_limit", po::value<int>()->default_value(30),
                        "time limit for meta heuristics in seconds");

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
        tests = {0, ANNEAL_FLAG, HILL_FLAG};

    tsp::TSPLIB_Directory dir(vm["path"].as<std::string>());
    tsp::TSPLIB_Matrix mtx;
    timespec start, end;
    double real_time;
    std::string method;
    for (auto const &graph_pair : dir.graphs) {
        auto &graph = graph_pair.second;
        std::cout << std::endl << graph_pair.first << std::endl
                << format("optimal fitness= %", graph.optimal_fitness)
                << std::endl;
        for (int flag : tests) {
            method = "Christofides";
            std::string ewt = graph.load(mtx);

            typedef std::mt19937 Random;
            Random random(64236738);
            double time = vm["time_limit"].as<int>();

            tsp::DynCycle cycle;

            clock_gettime(CLOCK_REALTIME, &start);

            cycle.resize(mtx.size1());
            if (ewt == "EUC_2D" || ewt == "CEIL_2D") {
                tsp::christofides<tsp::TSPLIB_Matrix, tsp::DynCycle, double>
                (mtx, cycle, mtx.size1(), ewt, mtx.X.get(), mtx.Y.get());
            } else {
                tsp::christofides<tsp::TSPLIB_Matrix, tsp::DynCycle>
                (mtx, cycle, mtx.size1());
            }

            tsp::CycleWalker<tsp::TSPLIB_Matrix, Random>
            walker(mtx, cycle, random);
            tsp::TimeMonitor monitor(time);
            tsp::BoltzmannDistr<Random>
            distr(tsp::fitness(mtx, cycle) / mtx.size1(), 1e-300, random);

            if (flag && HILL_FLAG) {
                method += "+ hill_climb";
                tsp::hill_climb(walker, monitor);
            }

            if (flag && ANNEAL_FLAG) {
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

