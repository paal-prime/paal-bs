Practical Approximation Algorithms Library
==========================================

Prerequisites
-------------

Please make sure you have the following packages installed and that all -PATH
environmental variables are properly set.

+ `gcc 4.8`
+ `boost 1.52`
+ `doxygen`
+ `pdflatex`
+ `gtest`
+ entire `texlive` distribution (for required packages see TeX sources)

Compiling and running
---------------------

The most important commands are the following:

+ `make world` creates all standalone binaries and documents
  - `./gtest` runs attached unit tests
+ `make results` compiles and runs all attached experiments and generates TeX
  reports in results/ subdirectories
+ `make clean` cleans all binaries and intermediate files
+ `make distclean` cleans everything (undoes `make world`)

