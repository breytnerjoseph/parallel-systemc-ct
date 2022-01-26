
SYSTEMC_PREFIX = /nil/local/breytner/systemc-2.3.3
EIGEN_INC_DIR = /nil/local/breytner/eigen_install/
TBB_INCLUDE_PATH = /usr/include/tbb/
BOOST_INCLUDE_PATH = /opt/boost/include/
BOOST_LIB_PATH = /opt/boost/lib/

TARGET_ARCH     = $(shell uname -m | sed -e 's/x86_64.*/linux64/' -e 's/i.86.*/linux/')

SYSTEMC_INC_DIR = $(SYSTEMC_PREFIX)/include
SYSTEMC_LIB_DIR = $(SYSTEMC_PREFIX)/lib


CXX             = g++

INC_DIR = ../include
INC = -I$(INC_DIR)


# INTEL TBB
CPPFLAGS = -I$(TBB_INCLUDE_PATH)

# BOOST 

CPPFLAGS += -I$(SYSTEMC_INC_DIR) 
CPPFLAGS += -I$(EIGEN_INC_DIR) -I$(BOOST_INCLUDE_PATH) -I$(TBB_INCLUDE_PATH)
CPPFLAGS += $(INC)
CPPFLAGS +=  -MMD -MP -MF $(basename $@).d
# -std=gnu++1z
CXXFLAGS = -std=gnu++11 -Wfatal-errors  -g -Wall -Wpedantic -Wno-long-long -O3 \
			-DSC_INCLUDE_DYNAMIC_PROCESSES -Wno-deprecated-declarations

SRCS = $(wildcard ../src/*.cpp)
OBJS = $(SRCS:%.cpp=%.o)
DEPS = $(SRCS:%.cpp=%.d)

TARGET = libsync.a

MODULE = sync 


.PHONY: $(MODULE)
$(MODULE): $(TARGET)

.PHONY: clean
clean:
	$(RM) $(DEPS) $(OBJS) $(TARGET)

# Include dependency files
-include $(DEPS)

%.o: %.cpp $(filter-out %.d, $(MAKEFILE_LIST))
	$(CXX) -c $< -o $@ $(CPPFLAGS) $(CXXFLAGS) $(CXXEXTRAFLAGS)

# "user-friendly" output of make for commands the students may not
# know:

$(OBJS): $(INC_DIR)/*

%.a: $(OBJS)
	@echo "[$(MODULE)] creating static lib $@..."
	$(AT)ar cru $@ $^
	$(AT)ranlib $@

%.gch: % $(filter-out %.d, $(MAKEFILE_LIST))
	@echo "[$(MODULE)] pre-compiling $<..."
	$(AT)$(RM) $@
	$(AT)$(CXX) -x c++-header -c $< -o $@ $(CPPFLAGS) $(CXXFLAGS) $(CXXEXTRAFLAGS)

%.d: ;

