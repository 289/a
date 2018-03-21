SOLUTION_ROOT_DIR = ../..

SINGLE_THREAD = false
DEBUG_VERSION = true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/common/object_data_pool


include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

SLIB_NAME = libobjdata.a.1.0.0

GEN_SRC   = ./gen
INCLUDES += -I$(CURRENT_PROJECT_DIR) -I$(GEN_SRC)

LIBSRCS = \
		  $(wildcard $(GEN_SRC)/player/*.cc) \
		  obj_data.cpp \
		  obj_attr_packet.cpp \
		  player_attr_declar.cpp \
		  codec_obj_data.cpp \
		  player_data.cpp \
		  global_data_def.cpp


all : lib

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.lib.mk

lib : $(LIB_O)
	cd $(BUILD) && $(AR) crs $(SLIB_NAME) $(addprefix ../,$^)

