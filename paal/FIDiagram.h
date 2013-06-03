#ifndef PAAL_FIDIAGRAM_H_
#define PAAL_FIDIAGRAM_H_

#include <vector>
#include <ostream>  // NOLINT
#include "format.h"
#include "paal/Logger.h"

namespace paal
{
  /** @brief [implements Table] collects data for fitness/iteration diagram */
  struct FIDiagram
  {
    double optimum;
    FIDiagram(double _optimum) : optimum(_optimum)
    {
      //records.push_back(Record("optimum"));
      //records.back().logger.log(1);
    }

    struct Record
    {
      explicit Record(const std::string &_name) : name(_name) {}
      std::string name;
      ImprovementLogger logger;
    };

    std::vector<Record> records;

    /**
    * @brief collects logs over single algorithm execution
    * @param name algorithm label for the Record object
    * @param algo [implements Algo] executed algorithm
    */
    template<typename Algo> void test(const std::string &name, Algo algo)
    {
      records.push_back(Record(name));
      algo.run(records.back().logger);
    }

    /** @brief prints the logs in a human readable form */
    void dump(std::ostream &os)
    {
      os << "iteration, fitness, algorithm\n";
      for (Record & r : records)
      {
        for (auto & rr : r.logger.records)
          os << format("%, %, %\n", rr.iteration, rr.fitness, r.name);
        os << format("%, %, %\n", r.logger.iterations, r.logger.records.back().fitness, r.name);
      }
    }

  void reduce(double unit)
  {
    assert(records.size());
    size_t it = records[0].logger.iterations;
    unit *= it;
    for(Record & r : records)
    {
      assert(r.logger.records.size());
      std::vector<ImprovementLogger::Record> R2,
        &R = r.logger.records;
      size_t i = 0;
      double f = R[0].fitness;
      for(double x = 0; x<it; x += unit)
      {
        while(i<R.size() && R[i].iteration<=x)
          f = R[i++].fitness;
        R2.push_back(ImprovementLogger::Record(x,f));
      }
      R.swap(R2);
    }
  }

  void dump_tex(std::ostream &tex, const std::string &test_name)
  {
    assert(records.size());
    for(Record & r : records) assert(r.logger.records.size());
    size_t it = records.back().logger.iterations;
    tex << format(
      "\\pgfsetplotmarksize{0pt}\n"
      "\\begin{figure}\n"
      " \\centering\n"
      " \\caption{\\label{%}%},\n"
      " \\begin{tikzpicture}\n"
      " \\begin{axis}[\n"
      "   width=0.75\\textwidth,\n"
      "   scale only axis,\n"
      "   xlabel=iteration,\n"
      "   ylabel=fitness,\n"
      "   xmin=0,xmax=%]\n",test_name,test_name,it);
    for (Record & r : records)
    {
      tex <<
      "   \\addplot coordinates {\n";
      for (auto & rr : r.logger.records) tex << format(
      "     (%,%)\n",rr.iteration,rr.fitness);
      tex << format(
      "     (%,%)\n"
      "   };\n"
      "   \\addlegendentry{%}\n",it,r.logger.records.back().fitness,r.name);
    }
    tex <<
      " \\end{axis}\n"
      " \\end{tikzpicture}\n"
      "\\end{figure}\n";
  }
  };
} // namespace paal

#endif  // PAAL_FIDIAGRAM_H_
