SOLUTION_ROOT_DIR = ../../../..

SINGLE_THREAD = false
DEBUG_VERSION =	true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/gamed/utility_lib/BTLib/tests
UTILITY_LIB_DIR     = $(SOLUTION_ROOT_DIR)/gamed/utility_lib


include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

EXES_NAME = test_bts
BIN_DIR = $(CURRENT_PROJECT_DIR)
EXES = $(BIN_DIR)/$(EXES_NAME)

INCLUDES += -I./ -I$(SOLUTION_ROOT_DIR)/gamed/utility_lib/BTLib -I$(UTILITY_LIB_DIR)

BT_SLIB += $(SOLUTION_ROOT_DIR)/gamed/utility_lib/BTLib/libBTs.a $(SOLUTION_ROOT_DIR)/gamed/utility_lib/pugixml/libpugixml.a

EXESRCS = \
		  test_bts.cpp \
		  btlib_link.cpp


all : $(EXES)

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.exe.mk

$(EXES) : $(EXE_O)
	$(LD) $(LDFLAGS) -o $@ $(addprefix ./,$^) $(BT_SLIB)

