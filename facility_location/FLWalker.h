#ifndef FACILITY_LOCATION_FLWALKER_H_
#define FACILITY_LOCATION_FLWALKER_H_

#include <vector>

namespace facility_location
{

/*
	concept FacilitySet : std::forward_iterator<bool>
*/

/** @brief [implements Walker] */
template<typename Instance> struct FLWalker
{
	template<typename FacilitySet>
	FLWalker(const Instance &_instance, const FacilitySet &fs) :
		instance(_instance)
	{
		//FIXME: shall I require iterators or just operator[]?
		//FIXME: shall I put it into initialization list?
		current_set.assign(fs.begin(),fs.end());
		current_fitness_ = get_fitness(current_set);
	}

private:
	const Instance &instance;
	std::vector<bool> current_set, next_set;
	double current_fitness_, next_fitness_;

	double get_fitness(const std::vector<bool> &fs)
	{
		double fitness = 0;
		for(size_t f=0; f<fs.size(); ++f)
			if(fs[f]) fitness += instance(f);
		for(size_t c=0; c<instance.cities_count(); ++c)
		{
			double best_cost = std::numeric_limits<double>::infinity();
			for(size_t f=0; f<fs.size(); ++f) if(fs[f])
				best_cost = std::min(best_cost, instance(f,c));
			fitness += best_cost;
		}
		return fitness;
	}

public:
	double current_fitness(){ return current_fitness_; }
	double next_fitness(){ return next_fitness_; }
	
	template<typename Random>
		void prepare_step(double progress, Random &random)
	{
		//achievable: O(C log F)
		//browsing whole neighbourhood may be profitable
		//larger neighbourhood may be useful
		//TODO: optimize fitness calculation, assignment maintenance
		//TODO: add facility swap (apart from add/remove)
		//TODO: add some interesting step distribution
		next_set = current_set;
		size_t f = random()%next_set.size();
		next_set[f] = !next_set[f];
		f = random()%next_set.size();
		next_set[f] = !next_set[f];

		next_fitness_ = get_fitness(next_set);
	}

	void make_step()
	{
		current_set = next_set;
		current_fitness_ = next_fitness_;
	}

};

}  // namespace facility_location

#endif  // FACILITY_LOCATION_FLWALKER_H_
