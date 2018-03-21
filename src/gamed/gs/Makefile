include ../Rules.mk


EXESRCS = \
		  main.cpp 


OBJS          = build/o/*.o

SUBDIRS       = global obj scene netio netmsg player movement item $(EVENTSYS_SUBDIRS) $(TEMPLATE_SUBDIRS) $(CLIENT_PROTO) $(COMBAT_SUBDIR) $(SKILL_SUBDIR) $(TASK_SUBDIR) $(ACHIEVE_SUBDIR)
OBJ_OBJS      = obj/$(OBJS)
SCENE_OBJS    = scene/$(OBJS)
GLOBAL_OBJS   = global/$(OBJS)
NETIO_OBJS    = netio/$(OBJS)
NETMSG_OBJS   = netmsg/$(OBJS)
PLAYER_OBJS   = player/$(OBJS)
MOVEMENT_OBJS = movement/$(OBJS)
ITEM_OBJS     = item/$(OBJS)
SUBOBJ        = $(GLOBAL_OBJS) $(OBJ_OBJS) $(SCENE_OBJS) $(NETIO_OBJS) $(NETMSG_OBJS) $(PLAYER_OBJS) $(MOVEMENT_OBJS) $(TEMPLATE_OBJS) $(EVENTSYS_OBJS) $(ITEM_OBJS) $(COMBAT_OBJS) $(CLIENTPROTO_OBJS) $(SKILL_OBJS) $(TASK_OBJS) $(ACHIEVE_OBJS)

EVENTSYS_SUBDIRS = eventsys/src eventsys/area_event
EVENT_ROOT_OBJS  = eventsys/src/$(OBJS)
AREA_EVENT_OBJS  = eventsys/area_event/$(OBJS)
EVENTSYS_OBJS    = $(EVENT_ROOT_OBJS) $(AREA_EVENT_OBJS)

TEMPLATE_SUBDIRS = template/data_templ template/map_data template/extra_templ
DATA_TEMPL_OBJS  = template/data_templ/$(OBJS)
MAP_DATA_OBJS    = template/map_data/$(OBJS)
EXTRA_TEMPL_OBJS = template/extra_templ/$(OBJS)
TEMPLATE_OBJS    = $(DATA_TEMPL_OBJS) $(MAP_DATA_OBJS) $(EXTRA_TEMPL_OBJS)

CLIENT_PROTO     = $(CURRENT_PROJECT_DIR)/client_proto
CLIENTPROTO_OBJS = $(CURRENT_PROJECT_DIR)/client_proto/$(OBJS)

COMBAT_SUBDIR    = $(CURRENT_PROJECT_DIR)/game_module/combat
COMBAT_OBJS      = $(CURRENT_PROJECT_DIR)/game_module/combat/$(OBJS)
SKILL_SUBDIR     = $(CURRENT_PROJECT_DIR)/game_module/skill
SKILL_OBJS       = $(CURRENT_PROJECT_DIR)/game_module/skill/$(OBJS)
TASK_SUBDIR      = $(CURRENT_PROJECT_DIR)/game_module/task
TASK_OBJS        = $(CURRENT_PROJECT_DIR)/game_module/task/$(OBJS)
ACHIEVE_SUBDIR   = $(CURRENT_PROJECT_DIR)/game_module/achieve
ACHIEVE_OBJS     = $(CURRENT_PROJECT_DIR)/game_module/achieve/$(OBJS)


all : $(EXES)

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.exe.mk

$(EXES) : $(EXE_O) $(SUBDIRS)
	$(LD) -fPIC $(LDFLAGS) -o $@ $(EXE_O) $(SUBOBJ) $(GAMED_STATIC_LIB) $(SHARED_STATIC_LIB) $(SYS_SHARE_LIB) $(PROTOBUF_LIB)

$(SUBDIRS): FORCE
	cd $@; make
	

FORCE:

clean: FORCE
	rm -rf $(CLEAN_BUILD) $(EXES);
	-($(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) clean;))

depend: FORCE
	make dep;
	-($(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) dep;))

