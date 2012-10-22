//Grzegorz Prusak
#ifndef tsp_Matrix_h
#define tsp_Matrix_h

#include <cassert>
#include <memory>

namespace tsp
{
	//square matrix with dynamically allocated memory
	template<typename T> struct DynMatrix
	{
		typedef T value_type;
		DynMatrix() : size_(0) {}

		value_type & operator()(size_t i, size_t j)
		{ return data_[i*size_+j]; }
		const value_type & operator()(size_t i, size_t j) const
		{ return data_[i*size_+j]; }
		
		void resize(size_t _size, size_t _size2)
		{
			assert(_size==_size2);
			if(size_==_size) return;
			data_.reset(new value_type[_size*_size]);
			size_ = _size;
		}

		size_t size1() const { return size_; }
		size_t size2() const { return size_; }

		size_t size_;
		std::unique_ptr<value_type[]> data_;
	};

	//static memory square matrix
	template<typename T, size_t _N> struct StaMatrix
	{
		typedef T value_type;
		enum { N = _N };
		
		value_type & operator()(size_t i, size_t j)
		{ return data_[i][j]; }
		const value_type & operator()(size_t i, size_t j) const
		{ return data_[i][j]; }

		void resize(size_t _size, size_t _size2)
		{
			assert(_size==_size2 && _size<=N);
			size_ = _size;
		}

		size_t size1() const { return size_; }
		size_t size2() const { return size_; }

		size_t size_;
		value_type data_[N][N];
	};	
}

#endif

