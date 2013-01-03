//checks how precise TimeAutoCtrl is and how many checks it performs
#include <iostream>
#include <random>
#include <unistd.h>
#include "format.h"
#include "paal/ProgressCtrl.h"

int main()
{
  enum { cfitness = 892342 };
  std::mt19937 random(417236);
  double start = paal::realtime_sec();
  int it = 0;
  paal::TimeAutoCtrl progress_ctrl(5);
  while (progress_ctrl.progress(cfitness) < 1)
  {
    it++;
    int i = random() % 100;
    if (!i) i = 10000;
    usleep(random() % i); //1ms on average
  }
  std::cout << format("it=%\n", it);
  double stop = paal::realtime_sec();
  std::cout << format("total time = %\ncheckpoints=%\n",
      stop - start, progress_ctrl.checkpoint_count);
  return 0;
}
