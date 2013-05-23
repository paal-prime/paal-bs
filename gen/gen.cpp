#include <cassert>
#include <cmath>

#include <iostream>
#include <fstream>
#include <random>
#include <string>

#include <boost/filesystem.hpp>

#include "tsp/util.h"
#include "facility_location/SimpleFormat.h"
#include "format.h"
//TODO: get Point class out of tsp

using namespace tsp;
using namespace facility_location;

std::mt19937 rnd(78682342);

double rnd_frac()
{ return double(rnd())/rnd.max(); }

Point rnd_rect(Point d)
{ return Point(rnd_frac()*(1-d.x),rnd_frac()*(1-d.y)); }

Point rnd_point(Point r, Point d)
{ return r + Point(rnd_frac()*d.x,rnd_frac()*d.y); }

/**
 * @param inst [output] generated instance
 * @param fc facilities count
 * @param cc cities count
 * @param oc_min minimal opening cost
 * @param oc_max maximal opening cost
 * @param rc rectangle count
 * @param dmin lower bounds on rectangle dimensions
 * @param dmax upper bounds on rectangle dimensions
 */

struct TestSpec
{
	size_t fc;
	size_t cc;
	double oc_min;
	double oc_max;
	size_t rc;
	Point dmin;
	Point dmax;


	void gen(std::ofstream &txt, std::ofstream &tex)
	{
		SimpleFormat<> inst;
		inst.optimal_cost_ = 1;
		inst.connecting_cost_.resize(fc,cc);
		inst.opening_cost_.resize(fc);
		for(auto &c : inst.opening_cost_)
			c = oc_min + (oc_max-oc_min)*rnd_frac();
		std::vector<tsp::Point> R(rc),D(rc),F(fc),C(cc);
		for(auto &p : D) p = rnd_point(dmin,dmax-dmin);
		for(size_t i=0; i<rc; ++i) R[i] = rnd_rect(D[i]);
		for(auto &p : F){ size_t i = rnd()%rc; p = rnd_point(R[i],D[i]); }
		for(auto &p : C){ size_t i = rnd()%rc; p = rnd_point(R[i],D[i]); }
		for(size_t f=0; f<fc; ++f)
			for(size_t c=0; c<cc; ++c)
				inst.connecting_cost_(f,c) = sqrt((F[f]-C[c]).sqr());
		txt << inst;
		tex <<
			"\\begin{figure}\n"
			"	\\centering\n"
			"	\\begin{tikzpicture}\n"
			"	\\begin{axis}[\n"
			"		width=0.75\\textwidth,\n"
			"		scale only axis,\n"
			"		xmin=0,xmax=1,ymin=0,ymax=1,\n"
			"		only marks]\n"
			"		\\addplot coordinates {\n";
		for(auto p : C) tex << format(
			"			(%, %)\n", p.x, p.y);
		tex <<
			"		};\n"
			"		\\addlegendentry{cities}\n"
			"		\\addplot coordinates {\n";
		for(auto p : F) tex << format(
			"			(%, %)\n", p.x, p.y);
		tex <<
			"		};\n"
			"		\\addlegendentry{facilities}\n"
			"	\\end{axis}\n"
			"	\\end{tikzpicture}\n"
			"\\end{figure}";
	}
};

/////////////////////////////////////

std::string dir_path = "FLClustered";

std::vector<TestSpec> T =
{
	{ 100, 1000, 5, 20, 30, Point(.1,.1), Point(.1,.1) },
	{ 100, 1000, 5, 20, 30, Point(.1,.1), Point(.1,.1) },
	{ 100, 1000, 5, 20, 30, Point(.1,.1), Point(.1,.1) }
};

int main()
{

	boost::filesystem::remove_all(dir_path);
	assert(boost::filesystem::create_directory(dir_path));
	for(size_t i=0; i<T.size(); ++i)
	{
		std::string txt_path = dir_path+format("/test%.txt",i);
		std::ofstream txt(txt_path);
		std::ofstream tex(dir_path+format("/test%.tex",i));
		txt << format("FILE: %\n",txt_path);
		T[i].gen(txt,tex);
	}
	return 0;
}

