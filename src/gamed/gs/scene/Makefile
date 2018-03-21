include ../../Rules.mk

INS_DIR = instance
BG_DIR  = battle

LIBSRCS = \
		  aoi.cpp \
		  slice.cpp \
		  grid.cpp \
		  world.cpp \
		  world_man.cpp \
		  world_man_lite.cpp \
		  world_cluster.cpp \
		  base_wm_imp.cpp \
		  $(INS_DIR)/ins_cluster.cpp \
		  $(INS_DIR)/ins_controller.cpp \
		  $(INS_DIR)/ins_worldman_imp.cpp \
		  $(INS_DIR)/ins_player_imp.cpp \
		  $(INS_DIR)/ins_script_if.cpp \
		  $(INS_DIR)/ins_func_table.cpp \
		  $(BG_DIR)/bg_cluster.cpp \
		  $(BG_DIR)/bg_controller.cpp \
		  $(BG_DIR)/bg_worldman_imp.cpp \
		  $(BG_DIR)/bg_player_imp.cpp \
		  $(BG_DIR)/bg_script_if.cpp \
		  $(BG_DIR)/bg_func_table.cpp \
		  $(BG_DIR)/bg_pve_rally.cpp \
		  $(BG_DIR)/bg_pve_worldboss.cpp \
		  $(BG_DIR)/bg_player_worldboss.cpp


all : lib
	
include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.lib.mk

lib : $(LIB_O)

