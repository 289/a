SOLUTION_ROOT_DIR = ../../..

SINGLE_THREAD = false
DEBUG_VERSION =	true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/logsrvd/logserver/src
PROTOCOL_DIR = $(SOLUTION_ROOT_DIR)/logsrvd/protocol
COMMON_DIR   = $(SOLUTION_ROOT_DIR)/logsrvd/common


include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

EXES_NAME = logsrvd
BIN_DIR = $(SOLUTION_ROOT_DIR)/logsrvd/logserver/bin
EXES = $(BIN_DIR)/$(EXES_NAME)

INCLUDES += -I$(CURRENT_PROJECT_DIR)
DEFINES  += -DFLUSH_EVERY_LINE

SYS_SHARE_LIB  = -lz

EXESRCS = \
		  main.cpp \
		  logsrv_server.cpp \
		  logsrv_session.cpp \
		  glogger.cpp \
		  worker_thread.cpp \
		  global_init.cpp \
		  $(COMMON_DIR)/glog_def.cpp \
		  $(PROTOCOL_DIR)/log_message.pb.cc


all : $(EXES)

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.exe.mk

$(EXES) : $(EXE_O)
	$(LD) $(LDFLAGS) -o $@ $(addprefix ./,$^) $(SHARED_STATIC_LIB) $(SYS_SHARE_LIB) $(EXTERNAL_SLIB)

