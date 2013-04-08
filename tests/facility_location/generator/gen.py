import os
from random import randrange, seed
from math import sqrt

RAND_SEED = 19912013
seed(RAND_SEED)

# Generates n random points in rectangle with
# opposite corners in (x1, y1) and (x2, y2).
def gen_points_in_rect(x1, y1, x2, y2, n):
  res = []
  for i in range(0, n):
    res.append((randrange(x1, x2), randrange(y1, y2)))
  return res

# Generates n random points in k rectangles contained in
# rectangle of side lengths x_size, y_size.
def gen_points(x_size, y_size, k, n):
  remaining = n
  points = []
  for i in range(0, k):
    x1 = randrange(0, x_size)
    x2 = randrange(x1 + 1, x_size + 1)
    y1 = randrange(0, y_size)
    y2 = randrange(y1 + 1, y_size + 1)
    n_p = int(remaining/(k - i))
    remaining -= n_p
    points.extend(gen_points_in_rect(x1, y1, x2, y2, n_p))
  return points

# Generates single test case, arguments are self-explanatory.
def gen_test(file_name, x_size, y_size, k, n, facility_cost_down, facility_cost_up, save_points = False):
  f = open(file_name + ".txt", "w+")
  points = set(gen_points(x_size, y_size, k, n))
  f.write("FILE: %s\n" % file_name)
  f.write("%d %d 0\n" % (len(points), len(points)))
  for i, pnt in enumerate(points, 1):
    (x, y) = pnt
    line = "%d %d " % (i, randrange(facility_cost_down, facility_cost_up))
    line += " ".join([str(int(sqrt((x - x_)**2 + (y - y_)**2))) for x_, y_ in points])
    line += '\n'
    f.write(line)
  f.close()
  if save_points:
    f2 = open(file_name + ".pts", "w+")
    for x, y in points:
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
  n_list = [100]
  k_list = [1, 5, 10, 20, 100]
  sizes = [100, 200, 500]
  for n in n_list:
    for k in k_list:
      for size in sizes:
        file_name = "%d_%d_%d" % (n, k, size)
        gen_test(os.path.join(uniform_dir, file_name), size, size, k, n, size, size + 1, save_points)
        gen_test(os.path.join(nonuniform_dir, file_name), size, size, k, n, size, 3*size, save_points)


if __name__ == '__main__':
  gen_tests("FLClustered")
