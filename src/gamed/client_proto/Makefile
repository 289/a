include ../Rules.mk

SOLUTION_ROOT_RELATIVE_PATH = ../..


# gs只需要编进C2G_proto.cpp，不要编G2C_proto.cpp
LIBSRCS = \
		  C2G_proto.cpp \
		  player_visible_state.cpp


all : lib
	
include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.lib.mk

lib : $(LIB_O)

