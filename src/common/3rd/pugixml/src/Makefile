SOLUTION_ROOT_DIR = ../../../..

SINGLE_THREAD = false
DEBUG_VERSION = true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/common/3rd/pugixml


include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

SLIB_NAME = libpugixml.a.1.2.0

INCLUDES += -I$(CURRENT_PROJECT_DIR) -I$(CURRENT_PROJECT_DIR)/inc

LIBSRCS =  \
		   pugixml.cpp


all : lib

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.lib.mk

lib : $(LIB_O)
	cd $(BUILD) && $(AR) crs $(SLIB_NAME) $(addprefix ../,$^)

