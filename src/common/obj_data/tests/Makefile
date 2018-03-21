BUILD_DIR_SOLUTION_ROOT_DIR = ../../../..
SOLUTION_ROOT_DIR = $(BUILD_DIR_SOLUTION_ROOT_DIR)

SINGLE_THREAD = false
DEBUG_VERSION = true


include ../../../shared/mk/gcc.defs.mk

DEFINES  += -g -ggdb

INCLUDES += -I./ 
SHARED_STATIC_LIB = $(SOLUTION_ROOT_DIR)/common/obj_data_pool/libobjdatapool.a 
SHARED_STATIC_LIB += $(SHARED_DIR)/net/libnetio.a $(SHARED_DIR)/logsys/liblogsys.a $(SHARED_DIR)/base/libsharedbase.a


TESTSRCS = \
		   serialize_test.cpp

include ../../../shared/mk/gcc.build.test.mk
