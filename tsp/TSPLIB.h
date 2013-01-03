#ifndef TSP_TSPLIB_H_
#define TSP_TSPLIB_H_

// http://www.iwr.uni-heidelberg.de/groups/comopt/software/TSPLIB95/

#include <boost/numeric/ublas/matrix.hpp>

#include <cassert>
#include <cmath>
#include <istream>  // NOLINT
#include <fstream>  // NOLINT
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "./format.h"

namespace tsp
{
  struct Point
  {
    Point() {}
    Point(double _x, double _y) : x(_x), y(_y) {}
    Point operator-(const Point &b) const
    {
      return Point(x - b.x, y - b.y);
    }
    double sqr() const
    {
      return x*x + y*y;
    }
    double x, y;
  };

  struct TSPLIB_Matrix  // implements Matrix
  {
    TSPLIB_Matrix() : size_(0) {}
    typedef double value_type;
    typedef int (*Dist)(Point);
    int operator()(size_t i, size_t j) const
    {
      return dist_ ? dist_(pos[i] - pos[j]) : mtx(i, j);
    }

    static int eucl_dist(Point d)
    {
      return int(.5 + sqrt(d.sqr()));  // NOLINT
    }
    static int ceil_dist(Point d)
    {
      return ceil(sqrt(d.sqr()));
    }
    static int att_dist(Point d)
    {
      return ceil(sqrt(d.sqr() / 10));
    }

    void resize(size_t _size, size_t _size2, Dist _dist = 0)
    {
      assert(_size == _size2);
      if ((dist_ = _dist))
      {
        pos.resize(_size);
        mtx.resize(0, 0);
      }
      else  // NOLINT
      {
        mtx.resize(_size, _size2);
        pos.clear();
      }
      size_ = _size;
    }

    size_t size1() const
    {
      return size_;
    }
    size_t size2() const
    {
      return size_;
    }

    Dist dist_;
    size_t size_;
    boost::numeric::ublas::matrix<int> mtx;
    std::vector<Point> pos;
  };

  struct TSPLIB_Directory
  {
    explicit TSPLIB_Directory(const std::string &dir)
    {
      std::ifstream index((dir + "/index").c_str());
      assert(index);
      double optimal_fitness;
      std::string header;
      while (get_header(index, header) >> optimal_fitness)
        graphs.push_back(
          Graph(dir + "/" + header + ".tsp", optimal_fitness));
    }

    struct Graph
    {
      Graph() {}
      Graph(const std::string &_filename, double _optimal_fitness) :
          filename(_filename), optimal_fitness(_optimal_fitness) {}
      std::string filename;
      double optimal_fitness;

      // http://www.iwr.uni-heidelberg.de/groups/comopt/software/TSPLIB95/TSPFAQ.html
      static double geo_rad(double x)
      {
        const double PI = 3.141592;
        int deg = x;
        return PI*(int(deg) + 5*(x - deg) / 3) / 180;  // NOLINT
      }

      static double geo_dist(Point a, Point b)
      {
        const double RRR = 6378.388;

        double q1 = cos(a.x - b.x);
        double q2 = cos(a.y - b.y);
        double q3 = cos(a.y + b.y);
        return int(RRR * acos(.5*((1. + q1)*q2 - (1. - q1)*q3)) + 1.0);  // NOLINT
      }

      void load(TSPLIB_Matrix &m)
      {
        std::ifstream is(filename.c_str());
        assert(is);
        expect_header(is, "DIMENSION");
        size_t n;
        assert(is >> n);
        expect_header(is, "EDGE_WEIGHT_TYPE");
        std::string ewt;
        assert(is >> ewt);
        if (ewt == "EXPLICIT")
        {
          expect_header(is, "EDGE_WEIGHT_FORMAT");
          std::string ewf;
          assert(is >> ewf);
          m.resize(n, n);
          expect(is, "EDGE_WEIGHT_SECTION");
          if (ewf == "FULL_MATRIX")
            for (size_t i = 0; i < n; ++i)
              for (size_t j = 0; j < n; ++j)
                assert(is >> m.mtx(i, j));
          else if (ewf == "UPPER_ROW")
          {
            for (size_t i = 0; i < n; ++i) m.mtx(i, i) = 0;
            for (size_t i = 0; i < n; ++i)
              for (size_t j = i + 1; j < n; ++j)
              {
                int d;
                assert(is >> d);
                m.mtx(i, j) = m.mtx(j, i) = d;
              }
          }
          else if (ewf == "LOWER_DIAG_ROW")  // NOLINT
            for (size_t i = 0; i < n; ++i)
              for (size_t j = 0; j <= i; ++j)
              {
                int d;
                assert(is >> d);
                m.mtx(i, j) = m.mtx(j, i) = d;
              }
          else if (ewf == "UPPER_DIAG_ROW")  // NOLINT
            for (size_t i = 0; i < n; ++i)
              for (size_t j = i; j < n; ++j)
              {
                int d;
                assert(is >> d);
                m.mtx(i, j) = m.mtx(j, i) = d;
              }
          else throw std::runtime_error(  //NOLINT
              format("EDGE_WEIGHT_FORMAT % is unimplemented", ewf));
        }
        else if (ewt == "GEO")  // NOLINT
        {
          expect(is, "NODE_COORD_SECTION");
          std::vector<Point> pos(n);
          int _;
          for (size_t i = 0; i < n; ++i)
            assert(is >> _ >> pos[i].y >> pos[i].x);
          for (Point &p : pos) p = Point(geo_rad(p.x), geo_rad(p.y));
          m.resize(n, n);
          for (size_t i = 0; i < n; ++i)
            for (size_t j = i; j < n; ++j)
              m.mtx(i, j) = m.mtx(j, i) = geo_dist(pos[i], pos[j]);
        }
        else  // NOLINT
        {
          if (ewt == "EUC_2D") m.resize(n, n, m.eucl_dist);
          else if (ewt == "CEIL_2D") m.resize(n, n, m.ceil_dist);
          else if (ewt == "ATT") m.resize(n, n, m.att_dist);
          else
            throw std::runtime_error(
              format("EDGE_WEIGHT_TYPE % is unimplemented", ewt));
          expect(is, "NODE_COORD_SECTION");
          int _;
          for (size_t i = 0; i < n; ++i)
            assert(is >> _ >> m.pos[i].x >> m.pos[i].y);
        }
      }
    };

    std::vector<Graph> graphs;

    static void expect(std::istream &is, const std::string &pattern)
    {
      std::string s;
      while (1)
      {
        assert(is >> s);
        if (s == pattern) return;
      }
    }
    static std::istream & get_header(std::istream &is, std::string &header)
    {
      std::getline(is, header, ':');
      std::stringstream ss(header);
      ss >> header;
      return is;
    }

    static void expect_header(std::istream &is, const std::string &header)
    {
      std::string s;
      while (1)
      {
        assert(get_header(is, s));
        if (s == header) return;
        assert(std::getline(is, s));
      }
    }
  };
}

#endif  // TSP_TSPLIB_H_

