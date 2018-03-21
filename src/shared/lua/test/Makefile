SOLUTION_ROOT_DIR = ../../..

SINGLE_THREAD = false
DEBUG_VERSION =	true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/shared/lua

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

TEST_DIR = $(CURRENT_PROJECT_DIR)/test
EXES_NAME = lua_test
BIN_DIR   = $(TEST_DIR)
EXES      = $(EXES_NAME)

INCLUDES += -I./ -I$(SOLUTION_ROOT_DIR) -I$(CURRENT_PROJECT_DIR) -I$(TEST_DIR)

EXESRCS = \
		  main.cpp

all : $(EXES)

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.exe.mk

$(EXES) : $(EXE_O)
	$(LD) $(LDFLAGS) -o $@ $(addprefix ./,$^) $(EXTERNAL_SLIB) ./libluaengine.a -llua -ldl

