//Grzegorz Prusak
#ifndef tsp_TSPLIB_h
#define tsp_TSPLIB_h

#include <cassert>
#include <cmath>
#include <memory>

namespace tsp
{
	struct TSPLIB_Matrix
	{
		typedef int value_type;
		int operator()(size_t i, size_t j) const
		{ return dist(X[i]-X[j],Y[i]-Y[j]); }
		
		static int eucl_dist(double xd, double yd)
		{ return int(.5+sqrt(xd*xd+yd*yd)); }
		static int ceil_dist(double xd, double yd)
		{ return ceil(sqrt(xd*xd+yd*yd)); }
		static int att_dist(double xd, double yd)
		{ return ceil(sqrt((xd*xd+yd*yd)/10)); }

		void resize(size_t _size, size_t _size2)
		{
			assert(_size==_size2);
			X.reset(new double[_size]);
			//badalloc below will invalidate the object
			Y.reset(new double[_size]);
			size_ = _size;
		}

		size_t size1() const { return size_; }
		size_t size2() const { return size_; }

		int (*dist)(double,double);
		size_t size_;
		std::unique_ptr<double[]> X,Y;
	};
}

#endif

