SOLUTION_ROOT_DIR = $(HOME)/svnproject/server/trunk/game_src
GAMED_ROOT  = $(SOLUTION_ROOT_DIR)/gamed

SINGLE_THREAD = false
DEBUG_VERSION =	true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/gamed


include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

EXES_NAME = gs
BIN_DIR   = $(CURRENT_PROJECT_DIR)/bin
EXES      = $(BIN_DIR)/$(EXES_NAME)

INCLUDES += -I./ -I$(GAMED_ROOT) -I$(GAMED_ROOT)/utility_lib/BTLib -I$(SOLUTION_ROOT_DIR)/common/protocol/gen

SYS_SHARE_LIB = -ldl
RPC_LIB       = $(SOLUTION_ROOT_DIR)/common/rpc/lib/libmasterrpc-pb.a
PROTOBUF_LIB  = $(EXTERNAL_SLIB) $(RPC_LIB) $(SOLUTION_ROOT_DIR)/common/protocol/lib/libgsproto-pb.a

LUA_ENGINE_LIB       = $(SOLUTION_ROOT_DIR)/shared/lua/libluaengine.a
SECURITY_LIB         = $(SOLUTION_ROOT_DIR)/shared/security/libsecurity.a
PUGIXML_LIB          = $(SOLUTION_ROOT_DIR)/common/3rd/pugixml/libpugixml.a
OBJ_DATA_POOL_LIB    = $(SOLUTION_ROOT_DIR)/common/obj_data/libobjdata.a
GLOG_CLIENT_LIB      = $(SOLUTION_ROOT_DIR)/logsrvd/logclient/lib/liblogclient.a
BEHAVIOR_TREE_LIB    = $(GAMED_ROOT)/utility_lib/BTLib/libBTs.a
COMPUTE_GEOMETRY_LIB = $(GAMED_ROOT)/utility_lib/CGLib/libCG.a
PATHFINDER_LIB       = $(GAMED_ROOT)/game_module/pathfinder/libpath.a
GAMED_STATIC_LIB     = $(OBJ_DATA_POOL_LIB) $(BEHAVIOR_TREE_LIB) $(PUGIXML_LIB) $(LUA_ENGINE_LIB) $(COMPUTE_GEOMETRY_LIB) $(SECURITY_LIB) $(PATHFINDER_LIB) $(GLOG_CLIENT_LIB)

USE_SELF_CLEAN_OPT   = true
