#Grzegorz Prusak
OBJ = walker_test.o performance/tsp/Cycle.o performance/tsp/annealing.o performance/tsp/various_seed.o
FLAGS = -I. -O2 --std=c++0x -Wall -Wshadow -pedantic #-D_XOPEN_SOURCE=600 -g

TEST_OBJ = tests/tsp/LazyCycle.o tests/tsp/TSPLIB.o
TEST_FLAGS = -lgtest -lgtest_main -lpthread

NO_COLOR = "\033[0m"
INFO_COLOR = "\033[96m"
WARNING_COLOR = "\033[93m"
OK_COLOR = "\033[92m"

all: $(OBJ) test
	g++ walker_test.o -lrt -o walker_test
	g++ performance/tsp/Cycle.o -lrt -o cycle_test
	g++ performance/tsp/annealing.o -o annealing_test
	g++ performance/tsp/various_seed.o -lrt -o various_seed_test

clean:
	rm -f $(OBJ) $(TEST_OBJ) *.d

rebuild: clean all

.PHONY: all clean rebuild

test: $(TEST_OBJ)
	g++ $^ $(TEST_FLAGS) -o test

%.o: %.cpp
	@echo $(INFO_COLOR)"compiling " $< $(NO_COLOR)
	g++ -c -MMD $(FLAGS) $< -o $@

-include $(OBJ:.o=.d) $(TEST_OBJ:.o=.d)

