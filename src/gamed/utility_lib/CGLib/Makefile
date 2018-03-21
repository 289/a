SOLUTION_ROOT_DIR = ../../..

SINGLE_THREAD = false
DEBUG_VERSION = true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/gamed/utility_lib/CGLib


include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

SLIB_NAME = libCG.a.1.0.0

INCLUDES += -I$(CURRENT_PROJECT_DIR)


LIBSRCS =  \
		   cglib.cpp


all : lib

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.lib.mk

lib : $(LIB_O)
	cd $(BUILD) && $(AR) crs $(SLIB_NAME) $(addprefix ../,$^)

