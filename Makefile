IGNORED_WARN := -Wno-vla -Wno-unused-parameter
OPTIMIZATIONS := -O2

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
# headers
HEADERS := $(shell find -name "*.h")

all: $(DEPENDS) $(MAIN) $(GTEST)

# include explicit dependencies
-include $(EXPLICITDEPS)

ifneq "$(MAKECMDGOALS)" "clean"
# include submakefiles
-include $(DEPENDS)
endif

CLEAN_DEPENDS := $(filter-out $(BLOSSOM5_DEPENDS), $(DEPENDS))

$(BLOSSOM5_DEPENDS) : %.d : %.cpp
	$(CXX) $(CXXFLAGS) -MT $(<:.cpp=.o) -MM $< > $@
	@echo -e "\t"$(CXX) --std=gnu++0x -w -c $(CFLAGS) $< -o $(<:.cpp=.o) >> $@

# create submakefiles
$(CLEAN_DEPENDS) : %.d : %.cpp
	$(CXX) $(CXXFLAGS) -MT $(<:.cpp=.o) -MM $< > $@
	@echo -e "\t"$(CXX) $(CXXFLAGS) -c $(CFLAGS) $< -o $(<:.cpp=.o) >> $@

# link main objects
$(MAIN) : % : %.o
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

# link gtest objects
$(GTEST) : % : $(GTESTOBJECTS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LDFLAGS_GTEST) -o $@

docs: $(HEADERS)
	doxygen doxygen.conf

clean:
	-rm -f *.o $(MAIN) $(GTEST) $(ALLOBJECTS) $(DEPENDS)
	-rm -rf $(DOCS_DIR)

.PHONY: clean

.SUFFIXES:
%: %.o
