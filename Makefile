#Grzegorz Prusak
OBJ =
FLAGS = -I. --std=c++0x -Wall -Wextra -Wshadow -pedantic -O2

TEST_OBJ = tests/tsp/SplayTree.o
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

