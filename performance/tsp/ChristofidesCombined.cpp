// Copyright 2013 <przed_deadlinem_zdazymy>
#include <boost/program_options.hpp>

#include <sys/time.h>
#include <random>
#include <string>
#include <iostream>
#include <vector>
#include <set>

//#include "tsp/annealing.h"
#include "tsp/Christofides.h"
//#include "tsp/CycleWalker.h"
//#include "tsp/monitor.h"
#include "tsp/TSPLIB.h"
#include "paal/ProgressCtrl.h"

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
                  //"2 - annealing, \n"
                  //"4 - hill climb \n"
                  "default: 1")
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
        tests = {CHRIST_FLAG };

    std::set<std::string> cases;
    if (vm.count("cases")) {
      std::vector<std::string> cases_param =
          vm["cases"].as<std::vector<std::string> >();
      cases.insert(cases_param.begin(), cases_param.end());
    }

    tsp::TSPLIB_Directory dir(vm["path"].as<std::string>());
    tsp::TSPLIB_Matrix mtx;
    double start, end;
    double real_time;
    std::string method;
    for (auto &graph_pair : dir.graphs) {
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

            typedef std::vector<int> CycleT;
            CycleT cycle;

            start = paal::realtime_sec();

            cycle.resize(mtx.size1());

            if (flag & CHRIST_FLAG) {
              method += "Christofides";
              if (ewt == "EUC_2D" || ewt == "CEIL_2D" || ewt == "ATT") {
                  tsp::christofides<tsp::TSPLIB_Matrix, CycleT>
                    (mtx, cycle, mtx.size1(), ewt, &mtx.pos);
              } else {
                  tsp::christofides<tsp::TSPLIB_Matrix, CycleT >
                    (mtx, cycle, mtx.size1());
              }
            } else {
              tsp::cycle_shuffle(cycle, mtx.size1(), random);
            }

            /*tsp::CycleWalker<tsp::TSPLIB_Matrix, Random>
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
            }*/

            end = paal::realtime_sec();
            real_time = (long long) ((end - start) * 1e3);
            double best_fitness = tsp::fitness(mtx, cycle);
            std::cout << format("%: fitness=% (%) real_time=%",
                                method, best_fitness,
                                best_fitness / graph.optimal_fitness,
                                real_time)
                      << std::endl;
        }
    }
    return 0;
}

