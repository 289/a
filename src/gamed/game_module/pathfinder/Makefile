SOLUTION_ROOT_DIR = ../../..

SINGLE_THREAD   = false
DEBUG_VERSION   = true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/gamed/game_module/pathfinder

include ../../../shared/mk/gcc.defs.mk


EXES_NAME       = libpath.so
BIN_DIR         = $(CURRENT_PROJECT_DIR)/bin
EXES            = $(BIN_DIR)/$(EXES_NAME)

SLIB_NAME       = libpath.a

PF_INCLUDE_DIR  = $(CURRENT_PROJECT_DIR)/include
PF_ENGINE_DIR   = $(CURRENT_PROJECT_DIR)/pf_engine
PF_AGENT_DIR    = $(CURRENT_PROJECT_DIR)/pf_agent
GS_GLOBAL_DIR   = $(SOLUTION_ROOT_DIR)/gamed

SECURITY_LIB = $(SOLUTION_ROOT_DIR)/shared/security/libsecurity.a

INCLUDES += -I$(PF_INCLUDE_DIR) -I$(PF_ENGINE_DIR) -I$(PF_AGENT_DIR) -I$(GS_GLOBAL_DIR)
DEFINES  += -DPLATFORM_LINUX -Wno-deprecated

EXESRCS = \
		  $(PF_ENGINE_DIR)/graph.cpp \
		  $(PF_ENGINE_DIR)/dgraph.cpp \
		  $(PF_ENGINE_DIR)/path_track.cpp \
		  $(PF_ENGINE_DIR)/move_map.cpp \
		  $(PF_ENGINE_DIR)/map_astar.cpp \
		  $(PF_ENGINE_DIR)/pf_common.cpp \
		  $(PF_ENGINE_DIR)/pf_engine.cpp \
		  $(PF_ENGINE_DIR)/pf_centre.cpp \
		  $(PF_ENGINE_DIR)/global_router.cpp \
		  $(PF_AGENT_DIR)/wayfinder_agent.cpp \
		  $(PF_AGENT_DIR)/ramble_agent.cpp \
		  $(PF_AGENT_DIR)/chase_agent.cpp \
		  $(PF_AGENT_DIR)/path_finder.cpp

all : lib

include ../../../shared/mk/gcc.build.exe.mk

$(EXES) : $(EXE_O)
	$(LD) $(LDFLAGS) -fPIC -shared -o $@ $(addprefix ./,$^)

lib : $(EXE_O)
	cd $(BUILD) && $(AR) crs $(SLIB_NAME) $(addprefix ../,$^)
