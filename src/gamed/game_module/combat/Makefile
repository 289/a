include ../../Rules.mk

TEMPLATE_DIR	    = $(GAMED_ROOT)/gs/template
GAME_MODULE         = $(GAMED_ROOT)/game_module
COMBAT_INC_DIR      = ../combat/include
COMBAT_UNIT_DIR     = ../combat/unit
COMBAT_COMBAT_DIR   = ../combat/combat
COMBAT_AINPC_DIR    = ../combat/ainpc
COMBAT_AISCENE_DIR  = ../combat/aiscene
COMBAT_GLOBAL_DIR   = ../combat/global

INCLUDES += -I$(TEMPLATE_DIR) -I$(GAME_MODULE) -I$(COMBAT_INC_DIR) -I$(COMBAT_UNIT_DIR) -I$(COMBAT_COMBAT_DIR) -I$(COMBAT_AINPC_DIR) -I$(COMBAT_AISCENE_DIR) -I$(COMBAT_GLOBAL_DIR)
#DEFINES  += -D_COMBAT_DUMP

LIBSRCS = \
		  unit/object.cpp \
		  unit/combat_unit.cpp \
		  unit/combat_player.cpp \
		  unit/combat_npc.cpp \
		  unit/unit_state.cpp \
		  unit/prop_policy.cpp \
		  unit/pet_man.cpp \
		  combat/combat.cpp \
		  combat/combat_atb.cpp \
		  combat/combat_state.cpp \
		  global/combat_man.cpp \
		  global/combat_mq.cpp \
		  global/combat_msg.cpp \
		  global/global_drop.cpp \
		  global/mob_spawner.cpp \
		  global/mob_manager.cpp \
		  global/world_boss.cpp \
		  global/combat_if.cpp \
		  global/combat_def.cpp \
		  global/obj_query.cpp \
		  combat_obj_if.cpp \
		  ainpc/eluna.cpp \
		  aiscene/combat_lua_API.cpp \
		  aiscene/combat_scene_ai.cpp \
		  aiscene/combat_lstate_man.cpp

all : lib
	
include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.lib.mk

lib : $(LIB_O)

