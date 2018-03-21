SOLUTION_ROOT_DIR = ../..

SINGLE_THREAD = false
DEBUG_VERSION = true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/shared/lua

include ../mk/gcc.defs.mk

SLIB_NAME = libluaengine.a.1.0.0

INCLUDES += -I$(CURRENT_PROJECT_DIR)

LIBSRCS =  \
		   lua_value.cpp \
		   lua_engine.cpp


all : lib

include ../mk/gcc.build.lib.mk

lib : $(LIB_O)
	cd $(BUILD) && $(AR) crs $(SLIB_NAME) $(addprefix ../,$^)


