//Grzegorz Prusak
#ifndef tsp_io_h
#define tsp_io_h

#include <ostream>

namespace tsp
{
	template<typename Matrix>
		void matrix_print(const Matrix &m, std::ostream &os)
	{
		for(size_t i=0; i<m.size1(); ++i)
		{ for(size_t j=0; j<m.size2(); ++j) os << m(i,j) << ' '; os << '\n'; }
	}

	template<typename Cycle>
		void cycle_print(const Cycle &cycle, std::ostream &os)
	{
		for(size_t i=0; i<cycle.size(); ++i) os << cycle[i] << ' ';
		os << '\n';
	}
}

#endif
