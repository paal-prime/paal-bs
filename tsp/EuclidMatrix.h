#ifndef TSP_EUCLIDMATRIX_H_
#define TSP_EUCLIDMATRIX_H_

#include "tsp/util.h"
#include <cassert>
#include <cmath>
#include <vector>

namespace tsp
{
	/**
	 * @brief [implements Matrix]
	 * represents distances between vertices on a 2D euclidian plane.
	 */
	struct EuclidMatrix
	{
		/**
		 * @brief euclidian distance between pos_i and pos_j
		 */
		double operator()(size_t i, size_t j) const
		{ return sqrt((pos[i]-pos[j]).sqr()); }

		size_t size1() const { return pos.size(); }
		size_t size2() const { return pos.size(); }

		void resize(size_t _size, size_t _size2)
		{
			assert(_size==_size2);
			pos.resize(_size);
		}

		/**
		 * @brief generates a set of random points for the matrix from [0,1)^2
		 * @param _size number of points to generate
		 * @param random (Random concept)
		 */
		template<typename Random> void generate(size_t _size, Random &random)
		{
			pos.resize(_size);
			for(Point &p : pos) p = Point(
				static_cast<double>(random())/random.max(),
				static_cast<double>(random())/random.max());
			//for(Point &p : pos) std::cout << format("(%,%)\n",p.x,p.y);
		}

		/**
		 * @brief represented points
		 */
		std::vector<Point> pos;
	};
}

#endif  // TSP_EUCLIDMATRIX_H_
