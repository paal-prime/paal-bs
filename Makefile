IGNORED_WARN := -Wno-vla -Wno-unused-parameter
OPTIMIZATIONS := -O2 #-g -gdwarf-2

CXX := g++ -I ./
CXXFLAGS := -Wall -Wextra -Wshadow -pedantic -std=gnu++0x $(IGNORED_WARN) $(OPTIMIZATIONS)
LDFLAGS := -lrt -lboost_program_options -lboost_filesystem -lboost_system -lpthread
LDFLAGS_GTEST := -lgtest -lgtest_main

DOCS_DIR=./docs

# sources
SOURCES := $(shell find -name "*.cpp")
# objects containing main() definition
MAINOBJECTS := $(subst .cpp,.o,$(shell grep -l "int main" $(SOURCES)))
# objects to be compiled as gtest executables
GTESTOBJECTS := $(subst .cpp,.o,$(shell grep -l "gtest/gtest" $(SOURCES)))
# executables (linked from MAINOBJECTS)
MAIN := $(subst .o,,$(MAINOBJECTS))
# executables (linked from GTESTOBJECTS)
GTEST := gtest
# submakefiles
DEPENDS := $(subst .cpp,.d,$(SOURCES))
# all objects
ALLOBJECTS := $(subst .cpp,.o,$(SOURCES))
# intermediate objects
OBJECTS := $(filter-out $(MAINOBJECTS) $(GTESTOBJECTS),$(ALLOBJECTS))
# explicit dependencies
EXPLICITDEPS := $(shell find -name "*.dep")

all: $(DEPENDS) $(MAIN) $(GTEST)

# include explicit dependencies
-include $(EXPLICITDEPS)

# include submakefiles
-include $(DEPENDS)

# create submakefiles
$(DEPENDS) : %.d : %.cpp
	$(CXX) $(CXXFLAGS) -MT $(<:.cpp=.o) -MM $< > $@
	@echo -e "\t"$(CXX) $(CXXFLAGS) -c $(CFLAGS) $< -o $(<:.cpp=.o) >> $@

# link main objects
$(MAIN) : % : %.o
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

# link gtest objects
$(GTEST) : % : $(GTESTOBJECTS)
	$(CXX) $(GTESTOBJECTS) $(BLOSSOM5_OBJECTS) $(CXXFLAGS) $(LDFLAGS) $(LDFLAGS_GTEST) -o $@

docs:
	doxygen doxygen.conf

clean:
	-rm -f *.o $(MAIN) $(GTEST) $(ALLOBJECTS) $(DEPENDS)
	-rm -rf $(DOCS_DIR)

.PHONY: clean docs
