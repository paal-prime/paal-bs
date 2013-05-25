#ifndef PAAL_GRIDTABLE_H_
#define PAAL_GRIDTABLE_H_

#include <vector>
#include <iomanip>
#include <ostream>  // NOLINT(readability/streams)
#include <string>

#include "paal/Logger.h"
#include "./format.h"

namespace paal
{
  struct GridTable
  {
    std::vector<std::string> columns;

    struct Record
    {
      explicit Record(const std::string &_name) : name(_name) {}
      std::string name;
      std::vector<double> results;

      template<typename Algo> void test(Algo algo)
      {
        VoidLogger logger;
        results.push_back(algo.run(logger));
      }
    };

    std::vector<Record> records;

    void push_algo(std::string name)
    {
      records.push_back(Record(name));
    }

    void dump(std::ostream &os)
    {
      os << std::setw(15);
      for (auto s : columns) os << std::setw(8) << s;
      os << '\n';
      for (Record & r : records)
      {
        os << std::setw(15) << r.name;
        for (double res : r.results) os << std::setw(8) << res;
        os << '\n';
      }
      os << std::flush;
    }

    void dump_tex(std::ostream &os)
    {
      std::string hline = format("\\cline{1-%}", columns.size() + 1);
      os << "\\begin{tabular}[ht]{|l||";
      for (auto s : columns) os << "c|";
      os << "H}\n" << hline << "\n & ";
      for (auto s : columns) os << s << " & ";
      os << "\\\\ " << hline << hline << " \n";
      for (Record & r : records)
      {
        os << r.name << " &";
        for (double res : r.results) os << res << " & ";
        os << "\\\\ " << hline << "\n";
      }
      os << "\\end{tabular}";
      os << std::flush;
    }
  };
}  // namespace paal

#endif  // PAAL_GRIDTABLE_H_
