// check triangle inequality in TSPLIB explicit matrices
#include "tsp/TSPLIB.h"
#include <iostream>
#include <string>
#include "format.h"

int main()
{
  std::string path = "./TSPLIB/symmetrical";
  tsp::TSPLIB_Directory dir(path);
  tsp::TSPLIB_Matrix mtx;
for (auto & g : dir.graphs)
  {
    g.second.load(mtx);
    if (mtx.dist_) continue;
    bool ok = 1;
    for (size_t i = 0; i < mtx.size1(); ++i)
      for (size_t j = 0; j < mtx.size1(); ++j)
        for (size_t k = 0; k < mtx.size1(); ++k)
          if (mtx(i, j) > mtx(i, k) + mtx(k, j))
          {
            i = j = k = mtx.size1();
            ok = 0;
          }
    std::cout << format("% %", g.second.filename, ok ? "OK" : "INVALID") << std::endl;
  }
  return 0;
}
