#!/usr/bin/python
import sys
from optparse import OptionParser

def num (s):
  try:
    return int(s)
  except exceptions.ValueError:
    return float(s)

def generate_plot(x_axis, y_axis, title, data_points, output_file):
  with open(output_file, 'w') as out:
    out.write('\\begin{figure}\n')
    out.write('\t\\centering\n')
    out.write('\t\\begin{tikzpicture}\n')
    out.write('\t\t\\begin{axis}[legend cell align=left,\n\
                             legend pos=outer north east,\n\
                             xlabel=' + x_axis + ',\n\
                             ylabel=' + y_axis + ',\n\
                             title={' + title + '}]\n')

    for k in data_points.keys():
      out.write('\t\t\\addplot coordinates {\n')
      for (x,y) in data_points[k]:
        out.write('\t\t\t(' + x + ', ' + y + ')\n')

      out.write('\t\t};\n')
      out.write('\t\t\\addlegendentry{' + k + '}\n')

    out.write('\t\t\\end{axis}\n')
    out.write('\t\\end{tikzpicture}\n')
    out.write('\\end{figure}\n')

parser = OptionParser()
parser.add_option("-i", "--input", action="store", dest="input_file",
                  help="Input file in csv format, used to generate chart.")
parser.add_option("-o", "--output", action="store", dest="output_file", default="chart_out.tex",
                  help="Output file containing chart.")

(options, args) = parser.parse_args()

if options.input_file is None:
  print "Please provide input file as command line argument."
  sys.exit(1)

with open(options.input_file) as f:
  content = f.read().splitlines()

if len(content) < 2:
  print "Input file contains not enough data."
  sys.exit(1)

header_line = content[:1][0].split(',')
if len(header_line) <> 3:
  print "Parsing header line failed."
  sys.exit(1)

for i in range(len(header_line)):
  header_line[i] = header_line[i].strip()

[x_axis_description, y_axis_description, chart_title] = header_line

data_points_lines = content[1:]

data_points = dict()

for data_row in data_points_lines:
  tokens = data_row.split(',')
  for i in range(len(tokens)):
    tokens[i] = tokens[i].strip()

  [x, y, plot_id] = tokens

  if plot_id in data_points:
    data_points[plot_id].append((x,y))
  else:
    data_points[plot_id] = [(x, y)]


generate_plot(x_axis_description, y_axis_description, chart_title, data_points, options.output_file)
