#ifndef RSARRAY_RSARRAY_H_
#define RSARRAY_RSARRAY_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <memory>

namespace rsarray
{
  /**
   * @brief Reversible Segment Array
   * lazy analog of: http://www.hars.us/Papers/revi.pdf
   * implemented on arrays
   */
  template<typename T> struct RSArray
  {
    typedef T value_type;

    RSArray() : data_size(0), segments_size(0), segments_capacity(0) {}
    template<typename Cycle> RSArray(const Cycle &cycle) :
      data_size(0), segments_size(0), segments_capacity(0)
    {
      *this = cycle;
    }

    RSArray & operator=(RSArray &cycle)
    {
      return *this = (const RSArray&)cycle;
    }

    template<typename Cycle> RSArray & operator=(const Cycle &cycle)
    {
      resize(cycle.size());
      for (size_t i = 0; i < data_size; ++i) data_[i] = cycle[i];
      return *this;
    }

    void resize(size_t n)
    {
      if (n == data_size) return;
      data_.reset(new value_type[data_size = n]);
      // badalloc below will invalidate the object
      segments_.reset(new Segment[segments_capacity = sqrt(3 * n) + 2]);
      segments_size = 1;
      segments_[0] = Segment(0, n);
    }

    struct Segment
    {
      Segment() {}
      Segment(size_t _begin, size_t _end) : begin(_begin), end(_end) {}
      size_t begin, end;
      size_t size()
      {
        return begin < end ? end - begin : begin - end;
      }
      size_t get(size_t i)
      {
        return begin < end ? begin + i : begin - i - 1;
      }
      void reverse()
      {
        std::swap(begin, end);
      }
      bool split(ssize_t i, Segment &s)
      {
        if (i < 0 || ssize_t(size()) < i) return 0;
        s = Segment(begin, begin < end ? begin + i : begin - i);
        begin = s.end;
        return 1;
      }
    };

    /**
     * @brief reduces segments_ to [[0,data_size)]
     * O(data_size)
     */
    void reduce()
    {
      value_type A[data_size];
      for (size_t j = 0, s = 0; j < segments_size; ++j)
        for (size_t i = 0; i < segments_[j].size(); ++i)
          A[s++] = data_[segments_[j].get(i)];
      for (size_t i = 0; i < data_size; ++i) data_[i] = A[i];
      segments_size = 1;
      segments_[0] = Segment(0, data_size);
    }

    /**
     * @brief reverses [begin,end) segment of the cycle
     * amortized O(sqrt(data_size))
     */
    void reverse(size_t begin, size_t end)
    {
      assert(begin <= end && end <= data_size);
      if (begin == end) return;
      size_t s = data_size;
      if (segments_size + 2 > segments_capacity) reduce();
      size_t ei = 0, bi = segments_size + 2;
      for (size_t i = segments_size;;)
      {
        segments_[--bi] = segments_[--i];
        s -= segments_[bi].size();
        if (ei <= bi && segments_[bi].split(end - s, segments_[bi - 1]))
          ei = bi--;
        if (segments_[bi].split(begin - s, segments_[bi - 1])) break;
      }
      for (size_t i = bi; i < ei; ++i) segments_[i].reverse();
      while (bi < ei) std::swap(segments_[bi++], segments_[--ei]);
      segments_size += 2;
    }

    value_type & operator[](size_t i)
    {
      for (size_t j = 0, s = 0;; s += segments_[j++].size())
        if (i < s + segments_[j].size())
          return data_[segments_[j].get(i - s)];
    }

    const value_type & operator[](size_t i) const
    {
      for (size_t j = 0, s = 0;; s += segments_[j++].size())
        if (i < s + segments_[j].size())
          return data_[segments_[j].get(i - s)];
    }

    size_t size() const
    {
      return data_size;
    }

    size_t data_size, segments_size, segments_capacity;
    std::unique_ptr<Segment[]> segments_;
    std::unique_ptr<value_type[]> data_;
  };
}  // namespace rsarray

#endif  // RSARRAY_RSARRAY_H_

