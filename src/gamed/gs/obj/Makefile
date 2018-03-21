include ../../Rules.mk

SERVICE_DIR = service

LIBSRCS = \
		  object.cpp \
		  unit.cpp \
		  npc.cpp \
		  pet.cpp \
		  area.cpp \
		  creature.cpp \
		  monster_imp.cpp \
		  service_npc_imp.cpp \
		  npc_gen.cpp \
		  area_gen.cpp \
		  actsession.cpp \
		  npcsession.cpp \
		  ainpc.cpp \
		  aipolicy.cpp \
		  aiaggro.cpp \
		  aggrolist.cpp \
		  npc_sender.cpp \
		  npc_msg_handler.cpp \
		  matter.cpp \
		  matter_gen.cpp \
		  matter_sender.cpp \
		  matter_msg_handler.cpp \
		  mine_imp.cpp \
		  world_boss_imp.cpp \
		  faction.cpp \
		  $(SERVICE_DIR)/service_executor.cpp \
		  $(SERVICE_DIR)/service_man.cpp \
		  $(SERVICE_DIR)/service_provider.cpp


all : lib
	
include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.lib.mk

lib : $(LIB_O)

