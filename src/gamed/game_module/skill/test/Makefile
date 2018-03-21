SOLUTION_ROOT_DIR = ../../../..

SINGLE_THREAD = false
DEBUG_VERSION =	true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/gamed/game_module/skill

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

EXES_NAME = test
BIN_DIR = $(CURRENT_PROJECT_DIR)/bin
EXES = $(BIN_DIR)/$(EXES_NAME)

COMMON_DIR = $(SOLUTION_ROOT_DIR)/gamed/game_module/obj_if
INCLUDE_DIR = $(CURRENT_PROJECT_DIR)/include
SKILL_DIR = $(CURRENT_PROJECT_DIR)/skill
UTIL_DIR = $(CURRENT_PROJECT_DIR)/utility
TEMPL_DIR = $(CURRENT_PROJECT_DIR)/template
TEST_DIR = $(CURRENT_PROJECT_DIR)/test

INCLUDES += -I./ -I$(COMMON_DIR) -I$(INCLUDE_DIR) -I$(UTIL_DIR) -I$(TEMPL_DIR) -I$(SKILL_DIR) -I$(TEST_DIR)

SYS_SHARE_LIB = -ldl
SECURITY_LIB = $(SOLUTION_ROOT_DIR)/shared/security/libsecurity.a
PROTO_LIB = $(EXTERNAL_SLIB)

EXESRCS = \
		  main.cpp \
		  combat.cpp \
		  combat_player.cpp \
		  $(TEMPL_DIR)/base_skill_templ.cpp \
		  $(TEMPL_DIR)/skill_templ.cpp \
		  $(TEMPL_DIR)/skill_target.cpp \
		  $(TEMPL_DIR)/effect_templ.cpp \
		  $(TEMPL_DIR)/skill_templ_manager.cpp \
		  $(UTIL_DIR)/player_util.cpp \
		  $(UTIL_DIR)/target_util.cpp \
		  $(UTIL_DIR)/range_util.cpp \
		  $(UTIL_DIR)/redir_util.cpp \
		  $(UTIL_DIR)/expression.cpp \
		  $(UTIL_DIR)/skill_expr.cpp \
		  $(UTIL_DIR)/attack_util.cpp \
		  $(SKILL_DIR)/skill_info.cpp \
		  $(SKILL_DIR)/skill_wrapper.cpp \
		  $(SKILL_DIR)/skill_types.cpp \
		  $(SKILL_DIR)/buff_wrapper.cpp \
		  $(SKILL_DIR)/cooldown_wrapper.cpp \
		  $(SKILL_DIR)/buff.cpp \
		  $(SKILL_DIR)/cast_proc.cpp \
		  $(SKILL_DIR)/active_point.cpp \
		  $(SKILL_DIR)/logic_if.cpp \
		  $(SKILL_DIR)/logic_basic_attack.cpp \
		  $(SKILL_DIR)/logic_change_prop.cpp \
		  $(SKILL_DIR)/logic_control.cpp \
		  $(SKILL_DIR)/logic_taunt.cpp \
		  $(SKILL_DIR)/logic_normal_shield.cpp \
		  $(SKILL_DIR)/logic_rebound_shield.cpp \
		  $(SKILL_DIR)/logic_summon_golem.cpp \
		  $(SKILL_DIR)/logic_power_flag.cpp \
		  $(SKILL_DIR)/logic_power_golem.cpp \
		  $(SKILL_DIR)/logic_reset_cooldown.cpp \
		  $(SKILL_DIR)/logic_purify.cpp \
		  $(SKILL_DIR)/logic_life_chain.cpp \
		  $(SKILL_DIR)/logic_revise.cpp \
		  $(SKILL_DIR)/logic_revive.cpp \
		  $(SKILL_DIR)/logic_trigger_effect.cpp

all : $(EXES)

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.exe.mk

$(EXES) : $(EXE_O)
	$(LD) $(LDFLAGS) -o $@ $(addprefix ./,$^) $(SHARED_STATIC_LIB) $(SECURITY_LIB) $(SYS_SHARE_LIB) $(PROTO_LIB)

