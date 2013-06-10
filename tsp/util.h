#ifndef TSP_UTIL_H_
#define TSP_UTIL_H_

#include <cassert>
#include <cctype>
#include <algorithm>

namespace tsp
{
  /*
  concept Matrix
  {
  double operator()(size_t i, size_t j) const;  // M_ij
  size_t size1() const;  // 1st dimension size
  size_t size2() cosnt;  // 2nd dimension size

  double & operator()(size_t i, size_t j);
  size_t resize(size_t _size1, size_t _size2);  // resizes the matrix
  };

  concept Cycle
  {
    size_t operator[](size_t i) const;  // C_i
    size_t size() const;

    size_t & operator[](size_t i);
  };
  */

  /** @brief point on a 2D plane */
  struct Point
  {
    Point() {}
    Point(double _x, double _y) : x(_x), y(_y) {}
    Point operator-(const Point &b) const
    {
      return Point(x - b.x, y - b.y);
	}
    Point operator+(const Point &b) const
    {
      return Point(x + b.x, y + b.y);
    }
    /** @brief dot product with itself */
    double sqr() const
    {
      return x * x + y * y;
    }
    /** @brief coordinates */
    double x, y;
  };


  /**
   * @brief calculates cycle length
   * @tparam Matrix [implements Matrix] square matrix
   * @tparam Cycle [implelemts Cycle]
   * @return cycle length
   */
  template<typename Matrix, typename Cycle>
  double fitness(const Matrix &m, const Cycle &cycle)
  {
    assert(m.size1() == m.size2() && m.size1() == cycle.size());
    double f = m(cycle[cycle.size() - 1], cycle[0]);
    for (size_t i = 0; i < cycle.size() - 1; ++i)
      f += m(cycle[i], cycle[i + 1]);
    return f;
  }


  /**
   * @brief generates a random cycle from values [0,n)
   * @tparam Cycle [implements Cycle]
   * @tparam Random [implements Random]
   */
  template<typename Cycle, typename Random>
  void cycle_shuffle(Cycle &cycle, size_t n, Random &random)
  {
    cycle.resize(n);
    for (size_t i = 0; i < n; ++i)
    {
      size_t j = random() % (i + 1);
      cycle[i] = cycle[j];
      cycle[j] = i;
    }
  }

  /**
   * @brief reverses [l,r) cycle segment
   * @param cycle [implements Cycle]
   *
   * ASSUMPTION: l <= r <= cycle.size()
   */
  template<typename Cycle>
  void cycle_reverse(Cycle &cycle, size_t l, size_t r)
  {
    while (l < r) std::swap(cycle[l++], cycle[--r]);
  }

  /**
   * rotates cycle so that the first element is 0 and the second is
   * its lower neighbour
   * @param cycle [implements Cycle]
   */
  template<typename Cycle> void cycle_normalize(Cycle &cycle)
  {
    if (!cycle.size()) return;
    uint32_t i = 0;
    while (cycle[i]) i++;
    if (i)
    {
      cycle_reverse(cycle, 0, i);
      cycle_reverse(cycle, i, cycle.size());
      cycle_reverse(cycle, 0, cycle.size());
    }
    if (cycle.size() > 1 && cycle[1] > cycle[cycle.size() - 1])
      cycle_reverse(cycle, 1, cycle.size());
  }

  /** @brief represents a split of a cycle into 2 paths */
  struct Split
  {
    /** see: @ref generate */
    uint32_t begin, end;

    /**
     * @brief generates split of cycle [0,n) into 2 intervals
     * of length at least 2.
     *
     * resulting parts are [@ref begin, @ref end) and [@ref end, @ref begin)
     *
     * @ref begin < @ref end
     *
     * distribution is uniform
     *
     * expected length = n/3:
     * integral x(1-x) / integral x = 1/3
     *
     * @param n cycle size in nodes
     * @param random [implements Random]
     */
    template<typename Random> void generate(uint32_t n, Random &random)
    {
      assert(n > 3);
      begin = random() % n;
      end = begin + random() % (n - 3) + 2;
      if (end > n) std::swap(begin, end -= n);
    }
  };
}  // namespace tsp

#endif  // TSP_UTIL_H_

