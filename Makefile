IGNORED_WARN := -Wno-vla -Wno-unused-parameter
OPTIMIZATIONS := -O2

CXX := g++ -I ./
CXXFLAGS := -Wall -Wextra -Wshadow -pedantic -std=gnu++0x $(IGNORED_WARN) $(OPTIMIZATIONS)
LDFLAGS := -lrt
LDFLAGS_GTEST := -lgtest -lgtest_main

# sources
SOURCES := $(shell find -name "*.cpp")
# objects containing main() definition
MAINOBJECTS := $(subst .cpp,.o,$(shell grep -l "int main" $(SOURCES)))
# objects to be compiled as gtest executables
GTESTOBJECTS := $(subst .cpp,.o,$(shell grep -l "gtest/gtest" $(SOURCES)))
# executables (linked from MAINOBJECTS)
MAIN := $(subst .o,,$(MAINOBJECTS))
# executables (linked from GTESTOBJECTS)
GTEST := $(subst .o,,$(GTESTOBJECTS))
# submakefiles
DEPENDS := $(subst .cpp,.d,$(SOURCES))
# all objects
ALLOBJECTS := $(subst .cpp,.o,$(SOURCES))
# intermediate objects
OBJECTS := $(filter-out $(MAINOBJECTS) $(GTESTOBJECTS),$(ALLOBJECTS))

all: $(DEPENDS) $(MAIN) $(GTEST)

# create submakefiles
$(DEPENDS) : %.d : %.cpp
	$(CXX) $(CXXFLAGS) -MT $(<:.cpp=.o) -MM $< > $@
	@echo -e "\t"$(CXX) $(CXXFLAGS) -c $(CFLAGS) $< -o $(<:.cpp=.o) >> $@

# link main objects
$(MAIN) : % : %.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -s $(LDFLAGS) -o $@ $^

# link gtest objects
$(GTEST) : % : %.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -s $(LDFLAGS) $(LDFLAGS_GTEST) -o $@ $^

# include submakefiles
-include $(DEPENDS)

clean:
	-rm -f *.o $(MAIN) $(GTEST) $(ALLOBJECTS) $(DEPENDS)

.PHONY: clean
