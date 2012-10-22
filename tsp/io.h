//Grzegorz Prusak
#ifndef tsp_io_h
#define tsp_io_h

#include <cassert>
#include <cmath>
#include <istream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "format.h"

template<typename T> inline T sqr(T a){ return a*a; }

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
	
	struct TSPLib
	{
		TSPLib(const std::string &dir)
		{
			std::ifstream index((dir+"/index").c_str()); assert(index);
			double optimal_fitness;
			std::string header;
			while(get_header(index,header) >> optimal_fitness)
				graphs.push_back(
					Graph(dir+"/"+header+".tsp", optimal_fitness));
		}

		struct Graph
		{
			Graph(){}
			Graph(const std::string &_filename, double _optimal_fitness) :
				filename(_filename), optimal_fitness(_optimal_fitness) {}
			std::string filename;
			double optimal_fitness;

			//http://www.iwr.uni-heidelberg.de/groups/comopt/software/TSPLIB95/TSPFAQ.html
			static double geo_rad(double x)
			{
				const double PI = 3.141592;
				int deg = x;
				return PI * (int(deg) + 5*(x-deg)/3) / 180; 
			}

			static double geo_dist(double longitude1, double latitude1,
				double longitude2, double latitude2)
			{
			 	const double RRR = 6378.388;

			  	double q1 = cos(longitude1 - longitude2); 
			   	double q2 = cos(latitude1 - latitude2); 
			    double q3 = cos(latitude1 + latitude2); 
				return int(RRR * acos(.5*((1.+q1)*q2 - (1.-q1)*q3)) + 1.0);
			}

			template<typename Matrix> void load(Matrix &m)
			{
				std::ifstream is(filename.c_str()); assert(is);
				expect_header(is,"DIMENSION"); size_t n; assert(is >> n);
				expect_header(is,"EDGE_WEIGHT_TYPE");
				std::string ewt; assert(is >> ewt);
				if(ewt=="EXPLICIT")
				{
					expect_header(is,"EDGE_WEIGHT_FORMAT");
					std::string ewf; assert(is >> ewf);
					m.resize(n,n);
					expect(is,"EDGE_WEIGHT_SECTION");
					if(ewf=="FULL_MATRIX")
						for(size_t i=0; i<n; ++i) for(size_t j=0; j<n; ++j)
							assert(is >> m(i,j));
					else if(ewf=="UPPER_ROW")
					{	
						for(size_t i=0; i<n; ++i) m(i,i) = 0;
						for(size_t i=0; i<n; ++i) for(size_t j=i+1; j<n; ++j)
						{ int d; assert(is >> d); m(i,j) = m(j,i) = d; }
					}
					else if(ewf=="LOWER_DIAG_ROW")
						for(size_t i=0; i<n; ++i) for(size_t j=0; j<=i; ++j)
						{ int d; assert(is >> d); m(i,j) = m(j,i) = d; }
					else if(ewf=="UPPER_DIAG_ROW")
						for(size_t i=0; i<n; ++i) for(size_t j=i; j<n; ++j)
						{ int d; assert(is >> d); m(i,j) = m(j,i) = d; }
					else throw std::runtime_error(
						format("EDGE_WEIGHT_FORMAT % is unimplemented",ewf));
				}
				else if(ewt=="EUC_2D")
				{
					expect(is,"NODE_COORD_SECTION");
					std::vector<double> X(n), Y(n); int _;
					for(size_t i=0; i<n; ++i) assert(is >> _ >> X[i] >> Y[i]);
					m.resize(n,n);
					for(size_t i=0; i<n; ++i) for(size_t j=0; j<n; ++j)
						m(i,j) = floor(.5+sqrt(sqr(X[i]-X[j])+sqr(Y[i]-Y[j])));
				}
				else if(ewt=="CEIL_2D")
				{
					expect(is,"NODE_COORD_SECTION");
					std::vector<double> X(n), Y(n); int _;
					for(size_t i=0; i<n; ++i) assert(is >> _ >> X[i] >> Y[i]);
					m.resize(n,n);
					for(size_t i=0; i<n; ++i) for(size_t j=0; j<n; ++j)
						m(i,j) = ceil(sqrt(sqr(X[i]-X[j])+sqr(Y[i]-Y[j])));
				}
				else if(ewt=="ATT")
				{
					expect(is,"NODE_COORD_SECTION");
					std::vector<double> X(n), Y(n); int _;
					for(size_t i=0; i<n; ++i) assert(is >> _ >> X[i] >> Y[i]);
					m.resize(n,n);
					for(size_t i=0; i<n; ++i) for(size_t j=0; j<n; ++j)
						m(i,j) = ceil(sqrt((sqr(X[i]-X[j])+sqr(Y[i]-Y[j])/10)));
				}
				else if(ewt=="GEO")
				{
					expect(is,"NODE_COORD_SECTION");
					std::vector<double> X(n), Y(n); int _;
					for(size_t i=0; i<n; ++i) assert(is >> _ >> X[i] >> Y[i]);
					for(double &x : X) x = geo_rad(x);
					for(double &y : Y) y = geo_rad(y);
					m.resize(n,n);
					for(size_t i=0; i<n; ++i) for(size_t j=i; j<n; ++j)
						m(i,j) = m(j,i) = geo_dist(X[i],Y[i],X[j],Y[j]);
				}
				else throw std::runtime_error(
					format("EDGE_WEIGHT_TYPE % is unimplemented",ewt));
			}
		};

		std::vector<Graph> graphs;
		
		static void expect(std::istream &is, const std::string &pattern)
		{
			std::string s;
			while(1)
			{
				assert(is >> s);
				if(s==pattern) return;
			}
		}
		static std::istream & get_header(std::istream &is, std::string &header)
		{
			std::getline(is,header,':');
			std::stringstream ss(header); ss >> header;
			//std::cout << format("[%]\n",header);
			return is;
		}

		static void expect_header(std::istream &is, const std::string &header)
		{
			std::string s;
			while(1)
			{
				assert(get_header(is,s));
				if(s==header) return;
				assert(std::getline(is,s));
			}
		}
	};
}

#endif
