#Grzegorz Prusak
OBJ = TSP_gen.o
FLAGS = -g --std=c++0x -Wall -Wshadow -pedantic #-D_XOPEN_SOURCE=600 -g

NO_COLOR = "\033[0m"
INFO_COLOR = "\033[96m"
WARNING_COLOR = "\033[93m"
OK_COLOR = "\033[92m"

all: $(OBJ)
	g++ TSP_gen.o -o gen

clean:
	rm -f $(OBJ) *.d

rebuild: clean all

.PHONY: all clean rebuild

%.o: %.cpp
	@echo $(INFO_COLOR)"compiling " $< $(NO_COLOR)
	g++ -c -MMD $(FLAGS) $< -o $@

-include $(OBJ:.o=.d)

