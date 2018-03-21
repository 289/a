SOLUTION_ROOT_DIR = ../../..

SINGLE_THREAD = false
DEBUG_VERSION = true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/logsrvd/logclient/src
PROTOCOL_DIR = $(SOLUTION_ROOT_DIR)/logsrvd/protocol
COMMON_DIR   = $(SOLUTION_ROOT_DIR)/logsrvd/common


include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

SLIB_NAME = liblogclient.a.1.0.0

INCLUDES += -I$(CURRENT_PROJECT_DIR)

LIBSRCS =  \
		   logsrv_client.cpp \
		   logsrv_session.cpp \
		   glog.cpp \
		   $(COMMON_DIR)/glog_def.cpp \
		   $(PROTOCOL_DIR)/log_message.pb.cc


all : lib

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.lib.mk

lib : $(LIB_O)
	cd $(BUILD) && $(AR) crs $(SLIB_NAME) $(addprefix ../,$^)

