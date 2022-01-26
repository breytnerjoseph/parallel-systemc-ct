MODULE = tdf

SRCS = $(wildcard *.cpp)

TARGET = main

all: $(TARGET)

ROOT=../
include $(ROOT)/Makefile.common

EXTRALDLIBS += ../common/libcommon.a $(SYNC_LAYER_PATH)/lib/libsync.a
CXXEXTRAFLAGS = -I../common
CEXTRAFLAGS = -I.

$(TARGET): $(OBJS) $(EXTRALDLIBS)
	$(LD) $(ESOFT_OBJS) $(OBJS) -o $@ $(LDFLAGS) $(EXTRALDLIBS) $(LDLIBS)

.PHONY: ../common/libcommon.a
../common/libcommon.a:
	@cd ../common && $(MAKE) common

.PHONY: $(SYNC_LAYER_PATH)/lib/libsync.a
$(SYNC_LAYER_PATH)/lib/libsync.a:
	@cd $(SYNC_LAYER_PATH)/lib/ && $(MAKE) sync