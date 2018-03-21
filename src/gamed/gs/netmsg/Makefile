include ../../Rules.mk

SOLUTION_ROOT_RELATIVE_PATH = ../../..
CLIENT_PACKET_DIR = $(SOLUTION_ROOT_RELATIVE_PATH)/gamed/client_proto


LIBSRCS = \
		  link_msg_proc.cpp \
		  master_msg_proc.cpp \
		  send_to_link.cpp \
		  send_to_master.cpp


all : lib
	
include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.lib.mk

lib : $(LIB_O)

