SOLUTION_ROOT_DIR = ../../../..

SINGLE_THREAD = false
DEBUG_VERSION =	true

GAMED_ROOT    = $(SOLUTION_ROOT_DIR)/gamed


include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

EXES_NAME = aoi_test
EXES      = $(EXES_NAME)

INCLUDES += -I./ -I$(SOLUTION_ROOT_DIR)/gamed
AOI_SRC   = $(SOLUTION_ROOT_DIR)/gamed/gs/scene

COMPUTE_GEOMETRY_LIB = $(GAMED_ROOT)/utility_lib/CGLib/libCG.a

EXESRCS = \
		  $(AOI_SRC)/aoi.cpp \
		  aoi_test.cpp


all : $(EXES)

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.exe.mk

$(EXES) : $(EXE_O)
	$(LD) $(LDFLAGS) -o $@ $(addprefix ./,$^) $(SHARED_STATIC_LIB) $(COMPUTE_GEOMETRY_LIB)

