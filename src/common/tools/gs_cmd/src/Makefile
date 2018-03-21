SOLUTION_ROOT_DIR = ../../../..

SINGLE_THREAD = false
DEBUG_VERSION =	true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/common/tools/gs_cmd/src
PROTOCOL_DIR = $(SOLUTION_ROOT_DIR)/common/protocol/gen


include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

EXES_NAME = gs_cmd
BIN_DIR = $(SOLUTION_ROOT_DIR)/common/tools/gs_cmd/bin
EXES = $(BIN_DIR)/$(EXES_NAME)

INCLUDES += -I$(CURRENT_PROJECT_DIR) -I$(PROTOCOL_DIR)
SHARED_STATIC_LIB += $(SHARED_DIR)/security/libsecurity.a

SYS_SHARE_LIB  = -lz -ldl

EXESRCS = \
		  main.cpp \
		  gs_client.cpp \
		  gs_session.cpp \
		  loop_srv.cpp \
		  proc_cmd_input.cpp \
		  $(PROTOCOL_DIR)/global/gs_runtime_cmd.pb.cc


all : $(EXES)

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.exe.mk

$(EXES) : $(EXE_O)
	$(LD) $(LDFLAGS) -o $@ $(addprefix ./,$^) $(SHARED_STATIC_LIB) $(SYS_SHARE_LIB) $(EXTERNAL_SLIB)

