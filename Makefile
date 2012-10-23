#Grzegorz Prusak
OBJ =
FLAGS = -I. -g --std=c++0x -Wall -Wshadow -pedantic #-D_XOPEN_SOURCE=600 -g

TEST_OBJ = tests/tsp/LazyCycle.o tests/tsp/TSPLIB.o
TEST_FLAGS = -lgtest -lgtest_main -lpthread

NO_COLOR = "\033[0m"
INFO_COLOR = "\033[96m"
WARNING_COLOR = "\033[93m"
OK_COLOR = "\033[92m"

all: $(OBJ) test

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

