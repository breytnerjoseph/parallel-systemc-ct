
SYSTEMC_PREFIX = /nil/local/breytner/systemc-2.3.3
EIGEN_INC_DIR = /nil/local/breytner/eigen_install/
TBB_INCLUDE_PATH = /usr/include/tbb/
BOOST_INCLUDE_PATH = /opt/boost/include/
BOOST_LIB_PATH = /opt/boost/lib/

TARGET_ARCH     = linux64

SYSTEMC_INC_DIR = $(SYSTEMC_PREFIX)/include
SYSTEMC_LIB_DIR = $(SYSTEMC_PREFIX)/lib


SYNC_LAYER_PATH = ../../../layer/
SYNC_LAYER_INC_DIR = $(SYNC_LAYER_PATH)/include

COMMON_INC_PATH = ../../common/include


# LIBTOOL Setting
SHELL           = /bin/bash
LIBTOOL_DIR     = /usr/bin
LIBTOOL         = $(SHELL) $(LIBTOOL_DIR)/libtool

GXX             = g++
GCC             = gcc
LD              = g++

SC_LIB = -L$(SYSTEMC_LIB_DIR) -lsystemc -lm
LDLIBS +=  $(SC_LIB) -pthread -lboost_system -lboost_thread 
LDLIBS += -ltbb # -ltbbmalloc -ltbbmalloc_proxy  


FLAGS           = -g -flto -Wall -Wpedantic -Wno-long-long  -O3\
                 -DSC_INCLUDE_DYNAMIC_PROCESSES -Wno-deprecated-declarations \
				 -I$(TBB_INCLUDE_PATH) -I$(BOOST_INCLUDE_PATH) \
				 -I$(SYNC_LAYER_INC_DIR) \
                 -I$(SYSTEMC_INC_DIR) \
				 -I$(EIGEN_INC_DIR) \
				 -I$(COMMON_INC_PATH) -std=gnu++11 -lboost_system
CXXFLAGS = -std=gnu++11 -flto -g -Wall -Wpedantic -Wno-long-long -O3 \
            -DSC_INCLUDE_DYNAMIC_PROCESSES -L$(BOOST_LIB_PATH) -lboost_system -lboost_thread
LDFLAGS = $(CXXFLAGS)


SRCS = $(wildcard *.cpp )
OBJS=$(SRCS:.cpp=.o)


EXTRALDLIBS += $(SYNC_LAYER_PATH)/lib/libsync.a

# main
main: $(OBJS) $(EXTRALDLIBS)
	# $(LIBTOOL) --mode=link --tag=CXX g++ -o $@ $(OBJS) $(LDFLAGS) -I$(SYNC_LAYER_INC_DIR)
	$(LD) $(OBJS) $(EXTRALDLIBS) -o $@  $(LDFLAGS) $(LDLIBS) 
.cpp.o: $(OBJS)
	$(CXX) -c $(FLAGS) $<

# Make all object files dependent on the synchronization implementaion .h files
$(OBJS) : $(SYNC_LAYER_INC_DIR)* 
# Make all object files dependent on .h files
$(OBJS) : *.h 
$(OBJS) : $(COMMON_INC_PATH)/*.h

trace_record: 
	uftrace record main

trace_seq_graph:
	uftrace dump --flame-graph | flamegraph.pl > seq.svg

trace_par_graph:
	uftrace dump --flame-graph | flamegraph.pl > par.svg

%.o: %.c
	$(LIBTOOL) --mode=compile gcc -Wall -c $<

.PHONY: $(SYNC_LAYER_PATH)/lib/libsync.a
$(SYNC_LAYER_PATH)/lib/libsync.a:
	@cd $(SYNC_LAYER_PATH)/lib/ && $(MAKE) sync

clean:
	rm -f main *.o *.lo
	rm -rf .libs
