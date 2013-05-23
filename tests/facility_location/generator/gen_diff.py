# This script is used to generate n cities and k facilities.
# Cost of connecting is euclidean distance between points.

import os
from random import randrange, seed, choice
from math import sqrt

RAND_SEED = 19912013
seed(RAND_SEED)

# Generates a random point in given rectangle.
def gen_point_in_rect(rect):
  x1, x2, y1, y2 = rect
  return (randrange(x1, x2), randrange(y1, y2))

# Generates n random points in given rectangles.
def gen_points(rects, n):
  res = set()
  for i in range(0, n):
    rect = choice(rects)
    res.add(gen_point_in_rect(rect))
  return res

# Generates k rectangles contained in rectangle of
# size x_size and y_size.
def gen_rects(x_size, y_size, k):
  res = []
  for i in range(0, k):
    x1 = randrange(0, x_size)
    x2 = randrange(x1 + 1, x_size + 1)
    y1 = randrange(0, y_size)
    y2 = randrange(y1 + 1, y_size + 1)
    res.append((x1, x2, y1, y2))
  return res

# Generates single test case, arguments are self-explanatory.
def gen_test(file_name, x_size, y_size, k, n, facility_cost_down, facility_cost_up, save_points = False):
  f = open(file_name + ".txt", "w+")
  rects = gen_rects(x_size, y_size, k)
  facilities_n, cities_n = n
  cities = gen_points(rects, cities_n)
  facilities = gen_points(rects, facilities_n)
  f.write("FILE: %s\n" % file_name)
  f.write("%d %d 0\n" % (len(facilities), len(cities)))
  for i, pnt in enumerate(facilities, 1):
    (x, y) = pnt
    line = "%d %d " % (i, randrange(facility_cost_down, facility_cost_up))
    line += " ".join([str(int(sqrt((x - x_)**2 + (y - y_)**2))) for x_, y_ in cities])
    line += '\n'
    f.write(line)
  f.close()

  with open(file_name + "_f.data", "w+") as f:
    for (x,y) in facilities:
      f.write("%f %f\n" % (x/float(x_size),y/float(y_size)))
  with open(file_name + "_c.data", "w+") as f:
    for (x,y) in cities:
      f.write("%f %f\n" % (x/float(x_size),y/float(y_size)))
  with open(file_name + ".tex", "w+") as f:
    f.write(
r"""\begin{tikzpicture}[only marks, x=10cm, y=10cm]
	\draw plot[color=red,mark=*] file {%s_f.data};
	\draw plot[mark=*] file {%s_c.data};
	\draw[->] (0,0) -- coordinate (x axis mid) (1,0);
	\draw[->] (0,0) -- coordinate (y axis mid) (0,1);
\end{tikzpicture}""" % (file_name,file_name))
  if save_points:
    f2 = open(file_name + ".pts", "w+")
    for x, y in facilities:
      f2.write("%d %d\n" % (x, y))
    for x, y in cities:
      f2.write("%d %d\n" % (x, y))
    f2.close()


# Generates multiple test cases, arguments are self-explanatory.
def gen_tests(base_dir, save_points = False):
  uniform_dir = os.path.join(base_dir, "uniform")
  nonuniform_dir = os.path.join(base_dir, "nonuniform")
  try:
    os.makedirs(uniform_dir)
    os.makedirs(nonuniform_dir)
  except:
    pass
  n_list = [(100, 100), (100, 10000), (1000, 1000)]
  k_list = [1, 5, 10, 20, 100]
  sizes = [10000]
  for n in n_list:
    for k in k_list:
      for size in sizes:
        facilities_n, cities_n = n
        file_name = "%dx%d_%d" % (facilities_n, cities_n, k)
        gen_test(os.path.join(uniform_dir, file_name), size, size, k, n, size, size + 1, save_points)
        gen_test(os.path.join(nonuniform_dir, file_name), size, size, k, n, size, 3*size, save_points)


if __name__ == '__main__':
  gen_tests("FLClusteredDiff")
