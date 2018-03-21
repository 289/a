SOLUTION_ROOT_DIR = ../../../..

SINGLE_THREAD = false
DEBUG_VERSION =	true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/gamed/utility_lib/CGLib


include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

EXES_NAME = test_cgeo
BIN_DIR = $(CURRENT_PROJECT_DIR)/tests
EXES = $(BIN_DIR)/$(EXES_NAME)

INCLUDES += -I./ -I$(SOLUTION_ROOT_DIR)/gamed/utility_lib/CGLib

CG_SLIB += $(CURRENT_PROJECT_DIR)/libCG.a

EXESRCS = \
		  test_cgeo.cpp 


all : $(EXES)

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.exe.mk

$(EXES) : $(EXE_O)
	$(LD) $(LDFLAGS) -o $@ $(addprefix ./,$^) $(CG_SLIB)

