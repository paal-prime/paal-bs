IGNORED_WARN := -Wno-vla -Wno-unused-parameter
OPTIMIZATIONS := -O2

CXX := g++ -I ./
CXXFLAGS := -Wall -Wextra -Wshadow -pedantic -std=gnu++0x $(IGNORED_WARN) $(OPTIMIZATIONS)
LDFLAGS := -lgtest -lgtest_main -lrt

# sources
SOURCES := $(shell find -name "*.cpp")
# objects containing main() definition
MAINOBJECTS := $(subst .cpp,.o,$(shell grep -l "int main" $(SOURCES)))
# executables (linked from MAINOBJECTS)
ALL := $(subst .o,,$(MAINOBJECTS))
# submakefiles
DEPENDS := $(subst .cpp,.d,$(SOURCES))
# all objects
ALLOBJECTS := $(subst .cpp,.o,$(SOURCES))
# objects not containing main() definition
OBJECTS := $(filter-out $(MAINOBJECTS),$(ALLOBJECTS))

all: $(DEPENDS) $(ALL)

# create submakefiles
$(DEPENDS) : %.d : %.cpp
	$(CXX) $(CXXFLAGS) -MT $(<:.cpp=.o) -MM $< > $@
	@echo -e "\t"$(CXX) $(CXXFLAGS) -c $(CFLAGS) $< -o $(<:.cpp=.o) >> $@

# link objects
$(ALL) : % : %.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -s $(LDFLAGS) -o $@ $^

# include submakefiles
-include $(DEPENDS)

clean:
	-rm -f *.o $(ALL) $(ALLOBJECTS) $(DEPENDS)

.PHONY: clean
