#ifndef FACILITY_LOCATION_FLWALKER_H_
#define FACILITY_LOCATION_FLWALKER_H_

#include <limits>
#include <vector>
#include <boost/numeric/ublas/matrix.hpp>
#include "facility_location/util.h"

namespace facility_location
{
  /** finds 2 nearest facilities (from the set) to the given city */
  struct Best2 
  {
	ssize_t b1,b2; //facility indices
	double c1,c2; //facility opening costs
	template<typename Instance, typename FacilitySet>
	Best2(const Instance &instance, const FacilitySet &fs, size_t city)
	{
		b1 = b2 = -1; c1 = c2 = 0;
		for(size_t i=0; i<fs.size(); ++i) if(fs[i])
		{
			double c = instance(i,city);
			if(b2==-1 || c<c2){ b2 = i; c2 = c; }
			if(b1==-1 || c2<c1)
			{ std::swap(b1,b2); std::swap(c1,c2); }
		}
	}
  };

  /** calculates costs of the steps of the form:
   *    add facility
   *    delete facility
   *    swap 2 facilities
   */
  struct StepCosts
  {
	  std::vector<double> ins,del;
	  boost::numeric::ublas::matrix<double> cor;

	  template<typename Instance, typename FacilitySet>
	  StepCosts(const Instance &instance, const FacilitySet &fs)
	  {
		double inf = std::numeric_limits<double>::infinity();
		size_t n = fs.size();
		assert(n==instance.facilities_count());
		ins.clear(); del.clear(); cor.clear();
		ins.resize(n);
		del.resize(n);
		cor.resize(n,n);
		for(size_t i=0; i<n; ++i) for(size_t j=0; j<n; ++j) cor(i,j) = 0;

		size_t active = 0; for(size_t i=0; i<n; ++i) active += fs[i];
		double base_fitness = active ? fitness(instance,fs) : 0;

		for(size_t i=0; i<n; ++i)
			if(fs[i]) del[i] = -instance(i);
			else ins[i] = instance(i);

		for(size_t city=0; city<instance.cities_count(); ++city)
		{
			Best2 best(instance,fs,city);
			del[best.b1] += best.b2!=-1 ? best.c2 - best.c1 : 0;
			for(size_t i=0; i<n; ++i) if(!fs[i])
			{
				double c = instance(i,city);
				if(best.b1==-1) ins[i] += c; // c
				else if(c<best.c1)
				{
					ins[i] += c - best.c1; // c < a (< b)
					if(best.b2!=-1) cor(i,best.b1) += best.c1 - best.c2; // c < a < b
				}
				else if(best.b2==-1) cor(i,best.b1) += c - best.c1; // a < c
				else if(c<best.c2) cor(i,best.b1) += c - best.c2; // a < c < b
			}
		}

		for(size_t i=0; i<n; ++i) for(size_t j=0; j<n; ++j)
			if(fs[i] || !fs[j]) cor(i,j) = inf;
			else cor(i,j) += ins[i]+del[j]+base_fitness;
		
		for(size_t i=0; i<n; ++i) if(fs[i])
		{ ins[i] = inf; del[i] += active>1 ? base_fitness : inf; }
		else
		{ del[i] = inf; ins[i] += base_fitness; }
	  }
  };

  /** @brief [implements Walker] */
  template<typename Instance> struct FLApxWalker
  {
      template<typename FacilitySet>
      FLApxWalker(const Instance &_instance, const FacilitySet &fs) :
        instance(_instance)
      {
		assert(instance.facilities_count()==fs.size());
		current_set.resize(fs.size());
		bool empty = 1;
		for(size_t i=0; i<fs.size(); ++i)
			if(current_set[i] = fs[i]) empty = 0;
		current_fitness_ = empty ? std::numeric_limits<double>::infinity() :
			fitness(instance, current_set);
      }

    private:
      const Instance &instance;
	  ssize_t step_ins,step_del;
      double current_fitness_, next_fitness_;

    public:
      std::vector<bool> current_set;
      
	  double current_fitness()
      {
        return current_fitness_;
      }
      double next_fitness()
      {
        return next_fitness_;
      }

      template<typename Random>
      void prepare_step(double progress, Random &random)
      {
		size_t n = current_set.size();
		StepCosts sc(instance,current_set);
		next_fitness_ = current_fitness_;
		step_ins = step_del = -1;
		for(size_t i=0; i<n; ++i)
		{
			if(next_fitness_>sc.del[i]){ next_fitness_ = sc.del[i]; step_ins = -1; step_del = i; }
			if(next_fitness_>sc.ins[i]){ next_fitness_ = sc.ins[i]; step_ins = i; step_del = -1; }
		}
		for(size_t i=0; i<n; ++i) for(size_t j=0; j<n; ++j)
			if(next_fitness_>sc.cor(i,j)){ next_fitness_ = sc.cor(i,j); step_ins = i; step_del = j; }
      }

      void make_step()
      {
        current_fitness_ = next_fitness_;
      	if(step_ins!=-1) current_set[step_ins] = 1;
		if(step_del!=-1) current_set[step_del] = 0;
	  }
  };

}  // namespace facility_location

#endif  // FACILITY_LOCATION_FLWALKER_H_
