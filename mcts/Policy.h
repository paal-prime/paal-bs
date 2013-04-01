#ifndef MCTS_POLICY_H_
#define MCTS_POLICY_H_

namespace mcts {

/*
concept Policy
{
	typedef Payload;

	template<typename Node, typename State>
	bool expand(Node &, const State &, size_t iteration, size_t level);
	
	template<typename Node, typename State>
	size_t choose(Node &, const State &);

	template<typename Node, typename State>
	void update(Node &, const State &, size_t index, double fitness);
};
*/

struct ExpandVoid
{
	struct Payload {};
	template<typename Node, typename State>
	void update(Node &, const State &, size_t, double){}
};

/** @brief [partial mcts::Policy]
 * expands always
 */
struct ExpandAlways : ExpandVoid
{
	template<typename Node, typename State>
	bool expand(Node &, const State &, size_t, size_t){ return 1; }
};

/** @brief [partial mcts::Policy]
 * expands nodes with level below LevelMax (root is at level 0)
 */
template<size_t LevelMax> struct ExpandLevels : ExpandVoid
{	
	template<typename Node, typename State>
	bool expand(Node &, const State &, size_t, size_t level)
	{ return level<LevelMax; }
};

/** @brief [partial mcts::Policy]
 * expands when number of visits reaches VisitsMin
 */
template<size_t VisitsMin> struct ExpandVisits
{
	struct Payload { size_t visits = 0; };
	
	template<typename Node, typename State>
	bool expand(Node &node, const State &, size_t, size_t)
	{ return node().visits>=VisitsMin; }

	template<typename Node, typename State>
	void update(Node &node, const State &, size_t, double)
	{ node().visits++; }
};

/** @brief [implements mcts::Policy]
 * Entrusts Expand to implement Payload, update() and expand().
 * Chooses child at random.
 */
template<typename Random, typename Expand> struct RandomPolicy : Expand
{
	RandomPolicy(Random &_random) : random(_random) {}
	Random &random;

	template<typename Node, typename State>
	size_t choose(Node &node, const State &)
	{ return random()%node.size(); }
};

/** @brief [implements mcts::Policy]
 * Entrusts Expand to implement Payload, update() and expand().
 * With probability eps chooses child at random, otherwise best child until now.
 */
template<typename Random, typename Expand> struct EpsBestPolicy : Expand
{
	EpsBestPolicy(double _eps) : eps(_eps){}
	double eps;

	struct Payload : typename Expand::Payload
	{ ssize_t index = -1; double fitness; };

	template<typename Node, typename State>
	size_t choose(Node &node, const State &)
	{
		if(node().index==-1 || random()<eps*random.max())
			return random()%node.size();
		return node().index;
	}

	template<typename Node, typename State>
	void update(Node &node, const State &state, size_t index, double fitness)
	{
		Expand::update(node,state,index,fitness);
		if(node().index==-1 || node().fitness>fitness)
		{
			node().best = index;
			node().fitness = fitness;
		}
	}
};

/** @brief [implements mcts::Policy]
 * Expands when there were at least visits_min visits.
 * With probability eps chooses random child.
 * Otherwise chooses an unvisited one, and when all have been visited at least
 * once, it chooses the one with best avg fitness
 */
template<typename Random> struct EpsAvgPolicy
{
	EpsAvgPolicy(double _eps, size_t _visits_min) :
		eps(_eps), visits_min(_visits_min) {}
	double eps;
	size_t visits_min;

	struct Payload
	{
		double fitness_sum = 0;
		size_t visits = 0;
		operator<(const Payload &b) const
		{ return !visits || fitness_sum*b.visits<b.fitness_sum*visits; }
	};
	
	template<typename Node, typename State>
	bool expand(Node &node, const State &, size_t, size_t)
	{ return node().visits>=visits_min; }
	
	template<typename Node, typename State>
	size_t choose(Node &node, const State &)
	{
		if(random()<eps*random.max()) return random()%node.size();
		size_t best = 0;
		for(size_t i=0; i<node.size(); ++i)
			if(node[i]<node[best]) best = i;
		return best;
	}

	template<typename Node, typename State>
	void update(Node &node, const State &state, size_t index, double fitness)
	{
		Expand::update(node,state,index,fitness);
		node().fitness_sum += fitness;
		node().visits++;
	}
};

}  // namespace mcts

#endif  // MCTS_POLICY_H_
