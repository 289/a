SOLUTION_ROOT_DIR = ../../../..

SINGLE_THREAD   = false
DEBUG_VERSION   = true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/gamed/game_module/pathfinder

include ../../../../shared/mk/gcc.defs.mk


EXES_NAME       = test
BIN_DIR         = $(CURRENT_PROJECT_DIR)/tests
EXES            = $(BIN_DIR)/$(EXES_NAME)

PF_ENGINE_DIR   = $(CURRENT_PROJECT_DIR)/pf_engine
PF_AGENT_DIR    = $(CURRENT_PROJECT_DIR)/pf_agent
PF_INCLUDE_DIR  = $(CURRENT_PROJECT_DIR)/include

INCLUDES += -I$(PF_INCLUDE_DIR) -I$(PF_ENGINE_DIR) -I$(PF_AGENT_DIR)
DEFINES  += -Wno-deprecated -DPLATFORM_LINUX

SHARED_SECURITY_LIB = $(SHARED_DIR)/security/libsecurity.a
SYS_SHARED_LIB = -lz -llua -lcrypto

EXESRCS = \
		  $(PF_ENGINE_DIR)/graph.cpp \
		  $(PF_ENGINE_DIR)/dgraph.cpp \
		  $(PF_ENGINE_DIR)/move_map.cpp \
		  $(PF_ENGINE_DIR)/path_track.cpp \
		  $(PF_ENGINE_DIR)/map_astar.cpp \
		  $(PF_ENGINE_DIR)/pf_common.cpp \
		  $(PF_ENGINE_DIR)/pf_engine.cpp \
		  $(PF_ENGINE_DIR)/pf_centre.cpp \
		  $(PF_ENGINE_DIR)/global_router.cpp \
		  $(PF_AGENT_DIR)/chase_agent.cpp \
		  $(PF_AGENT_DIR)/ramble_agent.cpp \
		  $(PF_AGENT_DIR)/wayfinder_agent.cpp \
		  $(PF_AGENT_DIR)/path_finder.cpp \
		  test.cpp

all : $(EXES)

include ../../../../shared/mk/gcc.build.exe.mk

#
#compile dynamic libaray
#$(EXES) : $(EXE_O)
#	$(LD) $(LDFLAGS) -L$(BIN_DIR) -lpath -o $@ $(addprefix ./,$^)
#

$(EXES) : $(EXE_O)
	$(LD) $(LDFLAGS) -o $@ $(addprefix ./,$^) $(SHARED_STATIC_LIB) $(SHARED_SECURITY_LIB) $(SYS_SHARED_LIB)
