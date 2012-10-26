//Grzegorz Prusak
#ifndef tsp_Cycle_h
#define tsp_Cycle_h

#include <memory>

namespace tsp
{
	struct DynCycle
	{
		typedef uint32_t value_type;
		DynCycle() : size_(0) {}
		DynCycle(const DynCycle &cycle) : size_(0) { *this = cycle; }
		
		operator value_type*(){ return data_.get(); }
		operator const value_type*() const { return data_.get(); }
	
		DynCycle & operator=(DynCycle &cycle)
		{ return *this = (const DynCycle&)cycle; }
		
		template<typename Cycle> DynCycle & operator=(const Cycle &cycle)
		{
			resize(cycle.size());
			for(size_t i=0; i<size_; ++i) data_[i] = cycle[i];
			return *this;
		}
		
		void resize(size_t _size)
		{
			if(size_==_size) return;
			data_.reset(new value_type[_size]);
			size_ = _size;
		}
		
		size_t size() const { return size_; }

		size_t size_;
		std::unique_ptr<value_type[]> data_;
	};

	template<size_t _N> struct StaCycle
	{
		typedef uint32_t value_type;
		enum { N = _N };
		void resize(size_t _size)
		{ assert(_size<=N); size_ = _size; }
		operator value_type*(){ return data_; }
		operator const value_type*() const { return data_; }
		size_t size() const { return size_; }
		size_t size_;
		value_type data_[N];
	};
}

#endif

