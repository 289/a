include ../../Rules.mk

LOOP_SRV_DIR    = loop_srv
GLOBAL_DATA_DIR = global_data

LIBSRCS = \
		  runnable_pool.cpp \
		  global_init.cpp \
		  timer.cpp \
		  inter_msg_queue.cpp \
		  dbgprt.cpp \
		  timed_task.cpp \
		  gmatrix.cpp \
		  cool_down.cpp \
		  randomgen.cpp \
		  math_types.cpp \
		  game_def.cpp \
		  obj_query.cpp \
		  combat_tick.cpp \
		  loop_srv.cpp \
		  glogger.cpp \
		  time_slot.cpp \
		  game_mall.cpp \
		  map_elem_ctrl.cpp \
		  game_lua_ent.cpp \
		  global_counter.cpp \
		  global_data.cpp \
		  drop_table.cpp \
		  auction_config.cpp \
		  achieve_config.cpp \
		  sys_mail_config.cpp \
		  team_util.cpp \
		  $(LOOP_SRV_DIR)/cmd_server.cpp \
		  $(LOOP_SRV_DIR)/cmd_session.cpp \
		  $(LOOP_SRV_DIR)/proc_runtime_cmd.cpp \
		  $(GLOBAL_DATA_DIR)/instance_record.cpp \
		  $(GLOBAL_DATA_DIR)/world_boss_record.cpp \
		  $(GLOBAL_DATA_DIR)/world_boss_info.cpp


all : lib
	
include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.lib.mk

lib : $(LIB_O)

