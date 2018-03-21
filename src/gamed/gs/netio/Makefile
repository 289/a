include ../../Rules.mk

SOLUTION_ROOT_RELATIVE_PATH = ../../..


LIBSRCS = \
		  master_client.cpp \
		  link_client.cpp \
		  network_evloop.cpp \
		  netio_if.cpp


all : lib
	
include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.lib.mk

lib : $(LIB_O)

