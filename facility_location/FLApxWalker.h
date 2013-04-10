#ifndef FACILITY_LOCATION_FLWALKER_H_
#define FACILITY_LOCATION_FLWALKER_H_

#include <vector>
#include <boost/numeric/ublas/matrix.hpp>
#include "facility_location/util.h"

namespace facility_location
{
  struct StepCosts
  {
	  double base_fitness;
	  std::vector<double> ins,del;
	  boost::numeric::ublas::matrix<double> cor;

	  double ins_fitness(size_t i){ return fitness_base+ins[i]; }
	  double del_fitness(size_t i){ return fitness_base+del[i]; }
	  double swap_fitness(size_t i, size_t d){ return fitness_base+ins[i]+cor(i,d)+del[i]; }

	  struct Best2
	  {
	    ssize_t b1,b2;
		double c1,c2;
		template<typename Instance, typename FacilitySet>
		Best2(const Instance &instance, const FacilitySet &fs, size_t city)
		{
			b1 = b2 = -1;
			for(size_t i=0; i<fs.size(); ++i) if(fs[i])
			{
				double c = instance(i,city);
				if(b2==-1 || c<c2){ b2 = i; c2 = c; }
				if(b1==-1 || c1<c2)
				{ std::swap(b1,b2); std::swap(c1,c2); }
			}
		}
	  };

	  template<typename Instance, typename FacilitySet>
	  StepCosts(const Instance &instance, const FacilitySet &fs)
	  {
		size_t n = fs.size();
		assert(n==instance.facilities_count());
		ins.clear(); del.clear(); cor.clear();
		ins.resize(n);
		del.resize(n);
		cor.resize(n,n);
		
		size_t active = 0; for(size_t i=0; i<n; ++i) active += fs[i];
		if(active) base_fitness = fitness(instance,fs) : 0;

		for(size_t city=0; city<instance.cities_count(); ++city)
		{
			Best2 best(instance,fs,city);
			del[best.b1] += best.b2!=-1 ? best.c2 - best.c1 :
				std::numeric_limits<double>::infinity();
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
	  }
  };

  /** @brief [implements Walker] */
  template<typename Instance> struct FLApxWalker
  {
      template<typename FacilitySet>
      FLWalker(const Instance &_instance, const FacilitySet &fs) :
        instance(_instance)
      {
        //FIXME: shall I require iterators or just operator[]?
        //FIXME: shall I put it into initialization list?
        current_set.assign(fs.begin(), fs.end());
        current_fitness_ = fitness(instance, current_set);
      }

    private:
      const Instance &instance;
      std::vector<bool> current_set, next_set;
      double current_fitness_, next_fitness_;

    public:
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
        //achievable: O(C log F)
        //browsing whole neighbourhood may be profitable
        //larger neighbourhood may be useful
        //TODO: optimize fitness calculation, assignment maintenance
        //TODO: add facility swap (apart from add/remove)
        //TODO: add some interesting step distribution
        
		StepCosts sc(instance,current_set);
		
		//...
      }

      void make_step()
      {
        current_set = next_set;
        current_fitness_ = next_fitness_;
      }

  };

}  // namespace facility_location

#endif  // FACILITY_LOCATION_FLWALKER_H_
