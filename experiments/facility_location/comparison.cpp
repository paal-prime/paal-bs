#include <cassert>
#include <iostream>
#include <random>
#include <boost/filesystem.hpp>

#include "facility_location/FLWalker.h"
#include "facility_location/SimpleFormat.h"
#include "facility_location/PrimDualSchema.h"

#include "paal/search.h"
#include "paal/ProgressCtrl.h"
#include "paal/StepCtrl.h"
#include "paal/GridTable.h"

typedef facility_location::SimpleFormat<double> Instance;

struct FLSearch
{
    FLSearch() : random(273648) {}

    const Instance *instance;
    std::mt19937 random;

    template<typename Logger> double run(Logger &logger)
    {
        std::vector<bool> fs(instance->facilities_count());
for(auto v : fs) v = random()&1;
        facility_location::FLWalker<Instance> walker(*instance,fs);
        paal::TimeAutoCtrl progress_ctrl(.05);
        paal::HillClimb step_ctrl;
        paal::search(walker,random,progress_ctrl,step_ctrl,logger);
        return walker.current_fitness()/instance->optimal_cost();
    }
};

struct FL3Apx
{
    const Instance *instance;

    template<typename Logger> double run(Logger &logger)
    {
        facility_location::PrimDualSchema<Instance> solver(*instance);
        return solver().first/instance->optimal_cost();
    }
};

int main()
{
    FLSearch ls;
    FL3Apx apx;
    paal::GridTable table;
    table.push_algo("optimum");
    table.push_algo("local search");
    table.push_algo("3 apx");
    for(boost::filesystem::directory_iterator it("UflLib/Euclid/");
            it!=boost::filesystem::directory_iterator(); ++it)
        if(it->path().extension().native()==".txt")
        {
            Instance instance(it->path().native());
            ls.instance = apx.instance = &instance;
            table.records[0].results.push_back(instance.optimal_cost());
            table.records[1].test(ls);
            table.records[2].test(apx);
            std::cout << "done " << it->path().native() << std::endl;
        }
    table.dump(std::cout);
    return 0;
}
